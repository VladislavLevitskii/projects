namespace backend.Crawler
{
    public class ExecutionService
    {
        private readonly CrawlQueue _queue;

        public ExecutionService(CrawlQueue queue)
        {
            _queue = queue;
        }

        public void StartManualExecution(string webPageId)
        {
            _queue.Enqueue(webPageId);
        }
    }
}
