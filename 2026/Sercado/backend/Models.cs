using System.ComponentModel.DataAnnotations;
using System.Text.Json.Serialization;

namespace backend
{
    [GraphQLName("WebPage")]
    public class WebPageRecord
    {
        [Key]
        [ID]
        public string Identifier { get; set; } = Guid.NewGuid().ToString();

        public string Label { get; set; } = string.Empty;
        public string Url { get; set; } = string.Empty;
        public string Regexp { get; set; } = string.Empty;

        public Periodicity Periodicity { get; set; }
        public List<string> Tags { get; set; } = [];
        public bool Active { get; set; } = true;

        [GraphQLIgnore]
        public List<Node> Nodes { get; set; } = [];
    }

    [GraphQLName("Node")]
    public class Node
    {
        [Key]
        public string Url { get; set; } = string.Empty;

        public string? Title { get; set; }
        public DateTime? CrawlTime { get; set; }

        [GraphQLIgnore]
        public List<string> Links { get; set; } = [];

        public List<Node> GetLinks()
        {
            if (Links.Count == 0) return [];
            return Links.Select(url => new Node { Url = url }).ToList();
        }

        public List<WebPageRecord> Owner { get; set; } = [];
    }

    public class Execution
    {
        public string Id { get; set; } = Guid.NewGuid().ToString();
        public required string WebPageId { get; set; }
        public ExecutionStatus Status { get; set; }
        public DateTime? StartTime { get; set; }
        public DateTime? EndTime { get; set; }
        public int SitesCrawled { get; set; }
    }

    [JsonConverter(typeof(JsonStringEnumConverter))]
    public enum Periodicity { Minute, Hour, Day }

    [JsonConverter(typeof(JsonStringEnumConverter))]
    public enum ExecutionStatus { Running, Completed, Failed, Queued, Paused }
}
