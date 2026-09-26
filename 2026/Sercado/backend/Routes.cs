using backend;
using backend.Crawler;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;

public static class RouteExtensions
{
    public static void AddRoutes(WebApplication app)
    {
        var api = app.MapGroup("/api");

        // GET /api/websites - Paginated, Filtered, Sorted
        api.MapGet("/websites", async (
            AppDbContext context,
            [FromQuery] string? url,
            [FromQuery] string? label,
            [FromQuery] string? tag,
            [FromQuery] string? sortBy,
            [FromQuery] int page = 1,
            [FromQuery] int pageSize = 10) =>
        {
            var query = context.WebPages.AsNoTracking();

            if (!string.IsNullOrWhiteSpace(url))
                query = query.Where(w => w.Url.Contains(url));

            if (!string.IsNullOrWhiteSpace(label))
                query = query.Where(w => w.Label.Contains(label));

            if (!string.IsNullOrWhiteSpace(tag))
                query = query.Where(w => w.Tags.Contains(tag));

            var totalCount = await query.CountAsync();

            var websites = await query.AsNoTracking().ToListAsync();

            var records = websites
                .Select(w => new
                {
                    Website = w,
                    LastExecution = context.Executions
                        .AsNoTracking()
                        .Where(e => e.WebPageId == w.Identifier)
                        .OrderByDescending(e => e.StartTime == null)
                        .ThenByDescending(e => e.StartTime)
                        .FirstOrDefault()
                })
                .ToList();

            var sorted = sortBy switch
            {
                "url-desc" => records.OrderByDescending(r => r.Website.Url),
                "last-desc" => records.OrderByDescending(r => r.LastExecution?.StartTime ?? DateTime.MinValue),
                "last-asc" => records.OrderBy(r => r.LastExecution?.StartTime ?? DateTime.MinValue),
                _ => records.OrderBy(r => r.Website.Url)
            };

            var items = sorted
                .Skip((page - 1) * pageSize)
                .Take(pageSize)
                .Select(r => new
                {
                    r.Website.Identifier,
                    r.Website.Label,
                    r.Website.Url,
                    r.Website.Regexp,
                    Periodicity = r.Website.Periodicity.ToString().ToLower(),
                    r.Website.Tags,
                    r.Website.Active,
                    LastExecutionTime = r.LastExecution?.StartTime,
                    LastExecutionStatus = r.LastExecution?.Status.ToString().ToLower()
                });

            return Results.Ok(new { totalCount, page, pageSize, items });
        });

        // POST /api/websites
        api.MapPost("/websites", async (WebPageRecord record, AppDbContext context) =>
        {
            if (string.IsNullOrWhiteSpace(record.Identifier))
                record.Identifier = Guid.NewGuid().ToString();

            context.WebPages.Add(record);
            await context.SaveChangesAsync();

            return Results.Created($"/api/websites/{record.Identifier}", record);
        });

        // PUT /api/websites/{id}
        api.MapPut("/websites/{id}", async (string id, WebPageRecord updated, AppDbContext context) =>
        {
            var existing = await context.WebPages.FindAsync(id);
            if (existing is null) return Results.NotFound();

            existing.Label = updated.Label;
            existing.Url = updated.Url;
            existing.Regexp = updated.Regexp;
            existing.Periodicity = updated.Periodicity;
            existing.Tags = updated.Tags;
            existing.Active = updated.Active;

            await context.SaveChangesAsync();
            return Results.Ok(existing);
        });

        // DELETE /api/websites/{id} (cascades to executions and orphaned nodes)
        api.MapDelete("/websites/{id}", async (string id, AppDbContext context) =>
        {
            var record = await context.WebPages.Include(w => w.Nodes).FirstOrDefaultAsync(w => w.Identifier == id);
            if (record is null) return Results.NotFound();

            var executions = context.Executions.Where(e => e.WebPageId == id);
            context.Executions.RemoveRange(executions);

            foreach (var node in record.Nodes.ToList())
            {
                node.Owner.Remove(record);
                if (node.Owner.Count == 0)
                {
                    context.Nodes.Remove(node);
                }
            }

            context.WebPages.Remove(record);
            await context.SaveChangesAsync();
            return Results.NoContent();
        });

        // GET /api/executions - Paginated, Filtered by websiteId
        api.MapGet("/executions", async (
            AppDbContext context,
            [FromQuery] string? websiteId,
            [FromQuery] int page = 1,
            [FromQuery] int pageSize = 10) =>
        {
            var query = context.Executions.AsNoTracking();

            if (!string.IsNullOrWhiteSpace(websiteId))
                query = query.Where(e => e.WebPageId == websiteId);

            var totalCount = await query.CountAsync();

            var items = await query
                .OrderByDescending(e => e.StartTime == null)
                .ThenByDescending(e => e.StartTime ?? DateTime.MinValue)
                .Skip((page - 1) * pageSize)
                .Take(pageSize)
                .Join(context.WebPages,
                      e => e.WebPageId,
                      w => w.Identifier,
                      (e, w) => new
                      {
                          e.Id,
                          e.WebPageId,
                          WebsiteLabel = w.Label,
                          Status = e.Status.ToString().ToLower(),
                          e.StartTime,
                          e.EndTime,
                          e.SitesCrawled
                      })
                .ToListAsync();

            return Results.Ok(new { totalCount, page, pageSize, items });
        });

        // POST /api/executions/{websiteId}/start
        api.MapPost("/executions/{websiteId}/start", async (string websiteId, CrawlQueue queue, AppDbContext context) =>
        {
            bool isRunning = await context.Executions
                .AnyAsync(e => e.WebPageId == websiteId && (e.Status == ExecutionStatus.Running || e.Status == ExecutionStatus.Queued));

            if (isRunning)
            {
                return Results.Conflict(new { message = "Execution already running or queued" });
            }

            var execution = new Execution
            {
                WebPageId = websiteId,
                Status = ExecutionStatus.Queued
            };

            context.Executions.Add(execution);
            await context.SaveChangesAsync();

            if (!queue.Enqueue(websiteId))
            {
                context.Executions.Remove(execution);
                await context.SaveChangesAsync();
                return Results.Conflict(new { message = "Execution already queued in memory" });
            }

            return Results.Accepted(value: new { message = "Execution queued", websiteId });
        });

        api.MapPost("/executions/{websiteId}/pause", async (string websiteId, AppDbContext context) =>
        {
            var execution = await context.Executions
                .FirstOrDefaultAsync(e => e.WebPageId == websiteId && e.Status == ExecutionStatus.Running);

            if (execution is null)
            {
                return Results.Conflict(new { message = "Execution is not currently running" });
            }

            execution.Status = ExecutionStatus.Paused;
            await context.SaveChangesAsync();

            return Results.Accepted(value: new { message = "Execution paused", websiteId });
        });
    }
}