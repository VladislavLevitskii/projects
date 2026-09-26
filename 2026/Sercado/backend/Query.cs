using HotChocolate;
using Microsoft.EntityFrameworkCore;

namespace backend
{
    public class Query
    {
        [GraphQLName("websites")]
        public IQueryable<WebPageRecord> GetWebsites(AppDbContext context)
        {
            return context.WebPages;
        }

        [GraphQLName("nodes")]
        public IQueryable<Node> GetNodes(AppDbContext context, [ID] List<string>? webPages)
        {
            IQueryable<Node> query = context.Nodes
                .Include(n => n.Owner)
                .AsNoTracking();

            if (webPages != null && webPages.Count > 0)
            {
                query = query.Where(node => node.Owner.Any(w => webPages.Contains(w.Identifier)));
            }

            return query;
        }
    }
}