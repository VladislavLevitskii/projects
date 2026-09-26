import type { ExecutionStatus, Periodicity, WebsiteRecord } from "./types";

export const PERIODICITY_OPTIONS = [
  { value: "minute", label: "Minute" },
  { value: "hour", label: "Hour" },
  { value: "day", label: "Day" },
];

export const PERIODICITY_MS: Record<Periodicity, number> = {
  minute: 60_000,
  hour: 3_600_000,
  day: 86_400_000,
};

export const STATUS_COLORS: Record<ExecutionStatus, string> = {
  queued: "gray",
  running: "blue",
  success: "green",
  failed: "red",
  paused: "yellow",
};

export const PAGE_SIZE = 6;

export const MAX_GRAPH_NODES = 1000;

export const initialWebsites: WebsiteRecord[] = [
  {
    id: "site-1",
    url: "https://example.com",
    boundaryRegexp: "^https?://(www\\.)?example\\.com",
    periodicity: "hour",
    label: "Example Docs",
    active: true,
    tags: ["docs", "public"],
  },
  {
    id: "site-2",
    url: "https://news.ycombinator.com",
    boundaryRegexp: "^https?://news\\.ycombinator\\.com",
    periodicity: "day",
    label: "Hacker News",
    active: true,
    tags: ["news"],
  },
  {
    id: "site-3",
    url: "https://mantine.dev",
    boundaryRegexp: "^https?://(www\\.)?mantine\\.dev",
    periodicity: "minute",
    label: "Mantine UI",
    active: false,
    tags: ["design", "ui"],
  },
];
