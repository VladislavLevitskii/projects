/* eslint-disable react-refresh/only-export-components -- context, hook and provider are kept in one file for readability */
import { createContext, useCallback, useContext, useEffect, useMemo, useState } from "react";
import type { Dispatch, SetStateAction } from "react";
import { MAX_GRAPH_NODES } from "../constants";
import type { GraphApiNode, GraphData, LiveMode, ViewMode } from "../types";
import { buildGraphData, graphResponseSchema } from "../utils/graph";
import { useExecutions } from "./ExecutionsContext";
import { useWebsites } from "./WebsitesContext";

export type GraphContextValue = {
  resolvedSelectedWebsiteIds: string[];
  viewMode: ViewMode;
  liveMode: LiveMode;
  graphData: GraphData;
  setSelectedWebsiteIds: Dispatch<SetStateAction<string[]>>;
  setViewMode: Dispatch<SetStateAction<ViewMode>>;
  setLiveMode: Dispatch<SetStateAction<LiveMode>>;
  refreshGraph: () => Promise<void>;
};

const GraphContext = createContext<GraphContextValue | null>(null);

export const useGraph = () => {
  const context = useContext(GraphContext);
  if (!context) {
    throw new Error("useGraph must be used within GraphProvider");
  }
  return context;
};

const EMPTY_GRAPH: GraphData = { nodes: [], edges: [] };

export const GraphProvider = ({ children }: { children: React.ReactNode }) => {
  const { websiteMap } = useWebsites();
  const { latestExecutions, refreshExecutions } = useExecutions();
  const [graphNodes, setGraphNodes] = useState<GraphApiNode[]>([]);
  const [selectedWebsiteIds, setSelectedWebsiteIds] = useState<string[]>([]);
  const [viewMode, setViewMode] = useState<ViewMode>("website");
  const [liveMode, setLiveMode] = useState<LiveMode>("live");

  const resolvedSelectedWebsiteIds = useMemo(
    () => selectedWebsiteIds.filter((id) => websiteMap.has(id)),
    [selectedWebsiteIds, websiteMap],
  );

  const tooManyNodes = useMemo(() => {
    if (viewMode !== "website") return false;
    const totalCrawled = selectedWebsiteIds.reduce(
      (sum, id) => sum + (latestExecutions.get(id)?.pagesCrawled ?? 0),
      0,
    );
    return totalCrawled > MAX_GRAPH_NODES;
  }, [viewMode, selectedWebsiteIds, latestExecutions]);

  const skipFetch = selectedWebsiteIds.length === 0 || tooManyNodes;

  const refreshGraph = useCallback(async () => {
    if (skipFetch) return;
    try {
      const res = await fetch("/graphql", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          query: `query GetNodes($webPages: [ID!]) {
            nodes(webPages: $webPages) {
              url
              title
              crawlTime
              links { url }
              owner { identifier }
            }
          }`,
          variables: { webPages: selectedWebsiteIds },
        }),
      });
      const result = graphResponseSchema.parse(await res.json());
      setGraphNodes(result.data.nodes);
    } catch (e) {
      console.error("Failed to fetch GraphQL nodes", e);
    }
  }, [skipFetch, selectedWebsiteIds]);

  useEffect(() => {
    refreshGraph();
  }, [refreshGraph]);

  // Static mode only turns off automatic polling so the graph doesn't shift while
  // it's being inspected. View mode and selection still apply immediately.
  useEffect(() => {
    if (liveMode !== "live") return;
    const interval = setInterval(() => {
      refreshExecutions();
      refreshGraph();
    }, 5000);
    return () => clearInterval(interval);
  }, [liveMode, refreshExecutions, refreshGraph]);

  // Nodes left over from before a fetch was skipped are not shown.
  const graphData = useMemo(
    () =>
      skipFetch ? EMPTY_GRAPH : buildGraphData(resolvedSelectedWebsiteIds, graphNodes, viewMode),
    [skipFetch, resolvedSelectedWebsiteIds, graphNodes, viewMode],
  );

  const value: GraphContextValue = useMemo(
    () => ({
      resolvedSelectedWebsiteIds,
      viewMode,
      liveMode,
      graphData,
      setSelectedWebsiteIds,
      setViewMode,
      setLiveMode,
      refreshGraph,
    }),
    [resolvedSelectedWebsiteIds, viewMode, liveMode, graphData, refreshGraph],
  );

  return <GraphContext.Provider value={value}>{children}</GraphContext.Provider>;
};
