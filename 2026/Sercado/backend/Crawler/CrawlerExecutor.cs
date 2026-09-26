using System.Collections.Concurrent;
using System.Text.RegularExpressions;
using Microsoft.EntityFrameworkCore;
using HtmlAgilityPack;

namespace backend.Crawler
{
    public class CrawlerExecutor
    {
        private readonly HttpClient _httpClient;
        private readonly IDbContextFactory<AppDbContext> _dbContextFactory;
        private readonly CrawlQueue _crawlQueue;
        private const int DegreeOfParallelism = 8;

        // Node.Url is joined to WebPageRecord through NodeWebPageRecord, whose primary key is
        // the composite (NodesUrl, OwnerIdentifier) - both nvarchar(450). SQL Server caps a
        // clustered index key at 900 bytes TOTAL across all columns of the key, not per column,
        // so Url alone can't safely use the full 450 chars: it must leave room for
        // OwnerIdentifier (a GUID string, 36 chars, unless a caller supplies a custom one).
        // Links longer than this (e.g. percent-encoded interlanguage links on Wikipedia
        // category pages) would fail to save and, since FlushBatchToDb dequeues a whole batch
        // before calling SaveChangesAsync, take the rest of that batch down with them. Drop
        // them at discovery time instead.
        private const int MaxUrlLength = 400;

        // Static in-memory lock shared across all incoming threads
        private static readonly ConcurrentDictionary<string, byte> _activeExecutions = new();

        public CrawlerExecutor(
            HttpClient httpClient, 
            IDbContextFactory<AppDbContext> dbContextFactory,
            CrawlQueue crawlQueue)
        {
            _httpClient = httpClient;
            _dbContextFactory = dbContextFactory;
            _crawlQueue = crawlQueue;
        }

        private static List<string> ExtractLinks(HtmlDocument doc, Uri baseUri)
        {
            List<string> links = new();
            HtmlNodeCollection? anchorNodes = doc.DocumentNode.SelectNodes("//a[@href]");

            if (anchorNodes is not null)
            {
                foreach (HtmlNode node in anchorNodes)
                {
                    var href = node.GetAttributeValue("href", "");
                    if (string.IsNullOrWhiteSpace(href) || href.StartsWith("#") || href.StartsWith("javascript:") || href.StartsWith("mailto:"))
                    {
                        continue;
                    }

                    if (Uri.TryCreate(baseUri, href, out Uri? absoluteUri))
                    {
                        if (absoluteUri.Scheme == Uri.UriSchemeHttp || absoluteUri.Scheme == Uri.UriSchemeHttps)
                        {
                            string path = absoluteUri.GetLeftPart(UriPartial.Path);
                            if (path.Length <= MaxUrlLength)
                            {
                                links.Add(path);
                            }
                        }
                    }
                }
            }
            return links.Distinct().ToList();
        }

        private async Task<(bool IsSuccess, string Title, List<string> Links)> CrawlWebsite(string currentUrl, int workerId)
        {
            try
            {
                Uri uri = new(currentUrl);
                Console.WriteLine($"[Crawler Worker {workerId} | Thread {Environment.CurrentManagedThreadId}] Requesting: {currentUrl}");

                using var request = new HttpRequestMessage(HttpMethod.Get, currentUrl);
                request.Headers.TryAddWithoutValidation("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
                request.Headers.TryAddWithoutValidation("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
                request.Headers.TryAddWithoutValidation("Accept-Language", "en-US,en;q=0.5");

                using HttpResponseMessage response = await _httpClient.SendAsync(request);
                if (!response.IsSuccessStatusCode)
                {
                    return (false, string.Empty, new());
                }

                string html = await response.Content.ReadAsStringAsync();
                HtmlDocument htmlDoc = new();
                htmlDoc.LoadHtml(html);

                string title = htmlDoc.DocumentNode.SelectSingleNode("//title")?.InnerText?.Trim() ?? "";
                title = System.Net.WebUtility.HtmlDecode(title);

                List<string> linksOnPage = ExtractLinks(htmlDoc, uri);
                return (true, title, linksOnPage);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Crawler Exception] {currentUrl}: {ex.Message}");
                return (false, string.Empty, new());
            }
        }

        private static async Task FlushBatchToDb(AppDbContext context, string webPageId, string executionId, int crawledCount, ConcurrentQueue<Node> pendingNodes)
        {
            if (pendingNodes.IsEmpty) return;

            List<Node> batch = new();
            while (pendingNodes.TryDequeue(out Node? node))
            {
                batch.Add(node);
            }

            if (batch.Count == 0) return;

            WebPageRecord? page = await context.WebPages.FirstOrDefaultAsync(w => w.Identifier == webPageId);
            if (page is null) return;

            var urls = batch.Select(b => b.Url).Distinct().ToList();
            var existingNodes = await context.Nodes
                .Include(n => n.Owner)
                .Where(n => urls.Contains(n.Url))
                .ToDictionaryAsync(n => n.Url);

            List<Node> newNodes = new();
            foreach (var node in batch.DistinctBy(n => n.Url))
            {
                if (existingNodes.TryGetValue(node.Url, out Node? existing))
                {
                    if (node.CrawlTime.HasValue)
                    {
                        existing.CrawlTime = node.CrawlTime;
                        existing.Title = node.Title;
                        existing.Links = node.Links;
                    }

                    if (!existing.Owner.Any(o => o.Identifier == page.Identifier))
                    {
                        existing.Owner.Add(page);
                    }
                }
                else
                {
                    node.Owner = [page];
                    newNodes.Add(node);
                }
            }

            if (newNodes.Count > 0)
            {
                context.Nodes.AddRange(newNodes);
            }

            var execution = await context.Executions.FirstOrDefaultAsync(e => e.Id == executionId);
            if (execution is not null)
            {
                execution.SitesCrawled = crawledCount;
            }

            await context.SaveChangesAsync();
        }

        public async Task ExecuteAsync(string webPageId, AppDbContext context)
        {
            // Atomically acquire the lock; abort immediately if another thread is already running this webPageId
            if (!_activeExecutions.TryAdd(webPageId, 0))
            {
                Console.WriteLine($"[Duplicate Blocked] An execution for {webPageId} is already running.");
                return;
            }

            Execution? execution = null;
            int sitesCrawledCount = 0;
            ConcurrentQueue<Node> pendingDbNodes = new();
            using var cts = new CancellationTokenSource();
            bool isPaused = false;
            Task? dbSyncTask = null;

            try
            {
                WebPageRecord? page = await context.WebPages.FirstOrDefaultAsync(w => w.Identifier == webPageId);
                if (page is null) return;

                execution = await context.Executions
                    .FirstOrDefaultAsync(e => e.WebPageId == page.Identifier && e.Status == ExecutionStatus.Queued);
                
                if (execution is not null)
                {
                    execution.Status = ExecutionStatus.Running;
                    execution.StartTime = DateTime.UtcNow;
                }
                else
                {
                    execution = new Execution
                    {
                        WebPageId = page.Identifier,
                        Status = ExecutionStatus.Running,
                        StartTime = DateTime.UtcNow
                    };
                    context.Executions.Add(execution);
                }

                await context.SaveChangesAsync();

                Regex regex = new(page.Regexp, RegexOptions.IgnoreCase | RegexOptions.Compiled);

                ConcurrentQueue<string> queue = new();
                ConcurrentDictionary<string, byte> visitedUrls = new();

                string normalizedStartUrl = page.Url;
                queue.Enqueue(normalizedStartUrl);
                visitedUrls.TryAdd(normalizedStartUrl, 0);

                int activeWorkers = 0;

                // Background task: Flushes new nodes to SQL Server every 1.5s so live graph polling works
                dbSyncTask = Task.Run(async () =>
                {
                    while (!cts.Token.IsCancellationRequested)
                    {
                        await Task.Delay(1000);
                        try
                        {
                            using var syncContext = await _dbContextFactory.CreateDbContextAsync();

                            var currentStatus = await syncContext.Executions
                                .Where(e => e.Id == execution.Id)
                                .Select(e => e.Status)
                                .FirstOrDefaultAsync();

                            if (currentStatus == ExecutionStatus.Paused)
                            {
                                isPaused = true;
                                cts.Cancel();
                                break;
                            }

                            await FlushBatchToDb(syncContext, page.Identifier, execution.Id, Volatile.Read(ref sitesCrawledCount), pendingDbNodes);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"[Live DB Sync Error]: {ex.Message}");
                        }
                    }
                });

                var workerTasks = Enumerable.Range(1, DegreeOfParallelism).Select(async workerId =>
                {
                    while (!cts.Token.IsCancellationRequested)
                    {
                        if (queue.TryDequeue(out string? currentUrl))
                        {
                            Interlocked.Increment(ref activeWorkers);
                            try
                            {
                                var sw = System.Diagnostics.Stopwatch.StartNew();
                                var crawlResults = await CrawlWebsite(currentUrl, workerId);
                                if (crawlResults.IsSuccess)
                                {
                                    pendingDbNodes.Enqueue(new Node
                                    {
                                        Url = currentUrl,
                                        CrawlTime = DateTime.UtcNow,
                                        Title = crawlResults.Title,
                                        Links = crawlResults.Links
                                    });

                                    Interlocked.Increment(ref sitesCrawledCount);

                                    foreach (string link in crawlResults.Links)
                                    {
                                        if (regex.IsMatch(link))
                                        {
                                            if (visitedUrls.TryAdd(link, 0))
                                            {
                                                queue.Enqueue(link);
                                            }
                                        }
                                        else
                                        {
                                            if (visitedUrls.TryAdd(link, 0))
                                            {
                                                pendingDbNodes.Enqueue(new Node
                                                {
                                                    Url = link,
                                                    CrawlTime = null,
                                                    Title = null,
                                                    Links = []
                                                });
                                            }
                                        }
                                    }
                                }

                                sw.Stop();
                                long elapsedMs = sw.ElapsedMilliseconds;
                                if (elapsedMs < 1000)
                                {
                                    await Task.Delay((int)(1000 - elapsedMs), cts.Token);
                                }
                            }
                            finally
                            {
                                Interlocked.Decrement(ref activeWorkers);
                            }
                        }
                        else
                        {
                            if (Volatile.Read(ref activeWorkers) == 0 && queue.IsEmpty)
                            {
                                cts.Cancel();
                                break;
                            }

                            await Task.Delay(30);
                        }
                    }
                });

                await Task.WhenAll(workerTasks);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Execution Failed]: {ex.Message}");
                if (execution is not null)
                {
                    execution.Status = ExecutionStatus.Failed;
                }
            }
            finally
            {
                cts.Cancel();
                if (dbSyncTask is not null)
                {
                    try { await dbSyncTask; } catch { }
                }

                if (execution is not null)
                {
                    try
                    {
                        using var finalContext = await _dbContextFactory.CreateDbContextAsync();
                        await FlushBatchToDb(finalContext, webPageId, execution.Id, sitesCrawledCount, pendingDbNodes);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[Final Flush Error]: {ex.Message}");
                        execution.Status = ExecutionStatus.Failed;
                    }

                    try
                    {
                        using var statusContext = await _dbContextFactory.CreateDbContextAsync();
                        var finalExec = await statusContext.Executions.FirstOrDefaultAsync(e => e.Id == execution.Id);
                        if (finalExec is not null)
                        {
                            finalExec.SitesCrawled = sitesCrawledCount;
                            finalExec.EndTime = DateTime.UtcNow;
                            if (isPaused)
                            {
                                finalExec.Status = ExecutionStatus.Paused;
                            }
                            else
                            {
                                finalExec.Status = execution.Status == ExecutionStatus.Failed ? ExecutionStatus.Failed : ExecutionStatus.Completed;
                            }
                            await statusContext.SaveChangesAsync();
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[Final Status Save Error]: {ex.Message}");
                    }
                }

                // Release both memory locks
                _activeExecutions.TryRemove(webPageId, out _);
                _crawlQueue.MarkComplete(webPageId);
            }
        }
    }
}