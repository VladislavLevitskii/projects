using System.Collections.Concurrent;

namespace backend.Crawler
{
    public class CrawlQueue
    {
        private readonly ConcurrentQueue<string> _queue = new();
        private readonly ConcurrentDictionary<string, byte> _activeOrQueued = new();

        public bool Enqueue(string webPageId)
        {
            if (string.IsNullOrWhiteSpace(webPageId)) return false;

            lock (_activeOrQueued)
            {
                if (_activeOrQueued.TryAdd(webPageId, 0))
                {
                    _queue.Enqueue(webPageId);
                    return true;
                }
            }

            return false;
        }

        public bool TryDequeue(out string? webPageRecordId)
        {
            return _queue.TryDequeue(out webPageRecordId);
        }

        public void MarkComplete(string webPageId)
        {
            if (!string.IsNullOrWhiteSpace(webPageId))
            {
                _activeOrQueued.TryRemove(webPageId, out _);
            }
        }
    }
}