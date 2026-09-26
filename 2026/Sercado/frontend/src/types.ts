import { z } from "zod";

export const periodicitySchema = z.enum(["minute", "hour", "day"]);
export type Periodicity = z.infer<typeof periodicitySchema>;

export type ExecutionStatus = "queued" | "running" | "success" | "failed" | "paused";
export type ViewMode = "website" | "domain";
export type LiveMode = "live" | "static";
export type PageView = "sites" | "executions" | "visualization";

export const websiteApiItemSchema = z.object({
  identifier: z.string(),
  url: z.string(),
  regexp: z.string(),
  periodicity: periodicitySchema,
  label: z.string(),
  active: z.boolean(),
  tags: z.array(z.string()).default([]),
});
export type WebsiteApiItem = z.infer<typeof websiteApiItemSchema>;

export const executionApiItemSchema = z.object({
  id: z.string(),
  webPageId: z.string(),
  status: z.enum(["queued", "running", "completed", "failed", "paused"]),
  startTime: z.string().nullable(),
  endTime: z.string().nullable().optional(),
  sitesCrawled: z.number().int().default(0),
});
export type ExecutionApiItem = z.infer<typeof executionApiItemSchema>;

export const graphApiOwnerSchema = z.object({
  identifier: z.string(),
})
export type GraphApiOwner = z.infer<typeof graphApiOwnerSchema>;

export const graphApiLinkSchema = z.object({
  url: z.string(),
});
export type GraphApiLink = z.infer<typeof graphApiLinkSchema>;

export const graphApiNodeSchema = z.object({
  url: z.string(),
  title: z.string().nullable().optional(),
  crawlTime: z.string().nullable().optional(),
  links: z.array(graphApiLinkSchema).default([]),
  owner: z.array(graphApiOwnerSchema).default([]),
});
export type GraphApiNode = z.infer<typeof graphApiNodeSchema>;

export type WebsiteRecord = {
  id: string;
  url: string;
  boundaryRegexp: string;
  periodicity: Periodicity;
  label: string;
  active: boolean;
  tags: string[];
};

export type Execution = {
  id: string;
  websiteId: string;
  status: ExecutionStatus;
  startTime: string | null;
  endTime?: string;
  pagesCrawled: number;
};

export type CrawlNode = {
  id: string;
  url: string;
  title?: string;
  crawlTime: string;
  links: string[];
};

export type CrawlSnapshot = {
  websiteId: string;
  executionId: string;
  nodes: CrawlNode[];
  generatedAt: string;
};

export type GraphNode = {
  id: string;
  url: string;
  label: string;
  crawled: boolean;
  crawlTime?: string;
  owners: string[];
  links?: string[];
};

export type GraphEdge = {
  id: string;
  source: string;
  target: string;
};

export type GraphData = {
  nodes: GraphNode[];
  edges: GraphEdge[];
};

export type GraphLayoutWorkerNode = {
  data: { id: string };
  position?: { x: number; y: number };
  locked?: boolean;
};

export type GraphLayoutWorkerMessage = {
  nodes: GraphLayoutWorkerNode[];
  edges: Array<{ data: GraphEdge }>;
  layoutIterations: number;
  // True when most nodes are locked and only a few newcomers need to ease
  // into an already-settled layout, as opposed to spreading a whole graph
  // out from scratch.
  incremental?: boolean;
};

export type WebsiteFormValues = {
  url: string;
  boundaryRegexp: string;
  periodicity: Periodicity;
  label: string;
  active: boolean;
  tags: string[];
};
