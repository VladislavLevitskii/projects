# WebCrawler Serĉado

Configurable web crawler that extracts data and maps out website structures. You set the starting URL and boundary rules, and the app builds an interactive graph showing exactly how the network is connected.

[Link to the 5-minute presentation video](https://www.youtube.com/watch?v=UC1Up_4li2M)

## Features

The project implements all the required core features and a few bonuses.

### Site and Execution Management
* **CRUD operations:** You can manage target websites. Each record has a URL, Boundary RegExp (to restrict crawling scope), Periodicity (minute, hour, day), Label, Tags, and an Active flag.
* **Dashboard:** The main view is paginated, sortable (by URL or last execution time), and can be filtered by URL, label, or tags. 
* **Execution queue:** Active sites are automatically queued for crawling based on their periodicity. You can also start executions manually.
* **Cascade delete:** Deleting a website record automatically cleans up its execution history and any orphaned nodes.

### Crawler (Executor)
* **Multi-threading:** The crawler runs in the background using multiple threads (C# Tasks). 
* **Rate limiting:** It enforces a maximum of 1 request per second per thread to avoid overloading target servers.
* **Data extraction:** For every allowed page, it extracts the URL, page `<title>`, crawl time, and all outgoing `<a>` links. 
* **Boundary checks:** Discovered links are only followed if their absolute URL matches the defined Boundary RegExp. 

### Visualization
* **Graph rendering:** Built with Cytoscape.js. It shows crawled pages as blue nodes and restricted boundary links as gray nodes.
* **View modes:** You can toggle between Website view (exact URLs) and Domain view (grouped by hostname).
* **Live updates:** In live mode, the graph polls for changes and updates automatically. As a bonus, the graph updates incrementally so the force-directed layout doesn't completely reset and mess up your current view.
* **Interactivity:** Double-clicking a crawled node shows its details and allows starting a new execution. Double-clicking an un-crawled boundary node lets you quickly create a new website record for it (which switches the app to live mode and adds it to the active selection).

### API
* **REST API:** Handles CRUD for websites and executions. Documented with OpenAPI/Swagger.
* **GraphQL:** Used to query the actual crawled data (nodes and their links) to prevent over-fetching.

## Tech Stack
* **Frontend:** React 19, TypeScript, Vite, Mantine UI, Cytoscape.js
* **Backend:** C# (.NET 10), Entity Framework Core, HotChocolate (for GraphQL)
* **Database:** MS SQL Server
* **Infrastructure:** Docker & Docker Compose

## How to run

The project uses Docker Compose, so you just need Docker installed.

1. Clone the repository:

```bash
git clone https://gitlab.mff.cuni.cz/teaching/nswi153/2025-26/team-06
cd webcrawler
```

2. Create a local `.env` file from the example and set a strong database password:

```bash
cp .env.example .env
```

The `.env` file is local-only and is ignored by Git.


3. Build and start the containers:

```bash
docker-compose up -d --build
```

4. Open the application:
* Frontend: http://localhost:5173
* GraphQL endpoint: http://localhost:9000/graphql
* Database Adminer: http://localhost:8081

> *Note*: Ports are dependant on `.env`

To stop the application, run `docker-compose down`.

## Notes

* **Regex performance:** The crawler evaluates every discovered link against your boundary regex on the backend. A poorly written regex might cause catastrophic backtracking and hang the crawling thread.
* **Bot protection:** The crawler uses a standard browser User-Agent, but heavily protected sites (like those using Cloudflare) might still return 403/429 errors. These executions will simply finish with fewer or zero crawled pages.
