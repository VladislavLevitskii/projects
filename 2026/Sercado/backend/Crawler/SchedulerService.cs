using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;

namespace backend.Crawler
{
    public class SchedulerService : BackgroundService
    {
        private readonly IServiceScopeFactory _scopeFactory;
        private readonly CrawlQueue _queue;

        public SchedulerService(IServiceScopeFactory scopeFactory, CrawlQueue queue)
        {
            _scopeFactory = scopeFactory;
            _queue = queue;
        }

        private async Task PlanNewCrawling(CancellationToken stoppingToken)
        {
            using (IServiceScope scope = _scopeFactory.CreateScope())
            {
                AppDbContext context = scope.ServiceProvider.GetRequiredService<AppDbContext>();

                var activePagesWithLastRun = await context.WebPages
                    .Where(w => w.Active)
                    .Select(w => new
                    {
                        WebPage = w,
                        LastExecutionEndTime = context.Executions
                            .Where(e => e.WebPageId == w.Identifier)
                            .OrderByDescending(e => e.EndTime)
                            .Select(e => e.EndTime)
                            .FirstOrDefault(),
                        ExecutionRunning = context.Executions
                            .Where(e => e.WebPageId == w.Identifier)
                            .Any(e => e.Status == ExecutionStatus.Running || e.Status == ExecutionStatus.Queued)
                    })
                    .ToListAsync(stoppingToken);

                foreach (var item in activePagesWithLastRun)
                {
                    WebPageRecord webPage = item.WebPage;
                    DateTime? lastRun = item.LastExecutionEndTime;
                    bool shouldCrawl = false;

                    if (lastRun is null)
                    {
                        shouldCrawl = true;
                    }
                    else
                    {
                        DateTime? nextRunTime = webPage.Periodicity switch
                        {
                            Periodicity.Minute => lastRun.Value.AddMinutes(1),
                            Periodicity.Hour => lastRun.Value.AddHours(1),
                            Periodicity.Day => lastRun.Value.AddDays(1),
                            _ => null
                        };

                        if (nextRunTime.HasValue && DateTime.UtcNow >= nextRunTime.Value)
                        {
                            shouldCrawl = true;
                        }
                    }

                    if (shouldCrawl && !item.ExecutionRunning)
                    {
                        _queue.Enqueue(webPage.Identifier);
                    }
                }
            }
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                await PlanNewCrawling(stoppingToken);

                while (_queue.TryDequeue(out string? webPageId))
                {
                    if (string.IsNullOrEmpty(webPageId))
                    {
                        continue;
                    }

                    _ = Task.Run(async () =>
                    {
                        using IServiceScope execScope = _scopeFactory.CreateScope();
                        AppDbContext execContext = execScope.ServiceProvider.GetRequiredService<AppDbContext>();
                        CrawlerExecutor executor = execScope.ServiceProvider.GetRequiredService<CrawlerExecutor>();

                        await executor.ExecuteAsync(webPageId, execContext);
                    }, stoppingToken);
                }

                // check every 10 seconds
                await Task.Delay(TimeSpan.FromSeconds(10), stoppingToken);
            }
        }
    }
}
