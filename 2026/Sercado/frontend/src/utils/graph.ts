import type { Core } from "cytoscape";
import { z } from "zod";
import { graphApiNodeSchema, type GraphApiNode, type GraphData, type GraphNode } from "../types";
import { getDomain } from "./url";

export const NODE_SIZE = 64;

// Newly added nodes get this class so they stay hidden (and, per cytoscape's
// behavior, so do their connected edges) until the layout worker has placed
// them, instead of flashing at their staged grid position first.
export const PENDING_LAYOUT_CLASS = "pending-layout";

export const graphResponseSchema = z.object({
  data: z
    .object({
      nodes: z.array(graphApiNodeSchema).default([]),
    })
    .default({ nodes: [] }),
});

export const buildGraphData = (
  websiteIds: string[],
  graphqlNodes: GraphApiNode[],
  viewMode: "website" | "domain",
): GraphData => {
  const nodeMap = new Map<string, GraphNode>();
  const edgeSet = new Set<string>();

  const resolveId = (url: string) => (viewMode === "domain" ? getDomain(url) : url);
  const resolveUrl = (url: string) => (viewMode === "domain" ? getDomain(url) : url);

  graphqlNodes.forEach((node) => {
    const owners = node.owner.map((owner) => owner.identifier);
    if (!owners.some((id) => websiteIds.includes(id))) {
      return;
    }

    const nodeId = resolveId(node.url);
    const nodeUrl = resolveUrl(node.url);
    const existing = nodeMap.get(nodeId);
    const links = node.links.map((link) => link.url);

    const nextOwners = existing?.owners ?? [];
    const updatedOwners = Array.from(new Set([...nextOwners, ...owners]));

    const normalizedCrawlTime = node.crawlTime ?? undefined;

    if (!existing) {
      nodeMap.set(nodeId, {
        id: nodeId,
        url: nodeUrl,
        label: viewMode === "domain" ? nodeUrl : (node.title ?? node.url),
        crawled: !!normalizedCrawlTime,
        crawlTime: normalizedCrawlTime,
        owners: updatedOwners,
        links,
      });
    } else {
      const existingTime = existing.crawlTime ? new Date(existing.crawlTime) : undefined;
      const nextTime = normalizedCrawlTime ? new Date(normalizedCrawlTime) : undefined;
      const shouldReplace =
        !existingTime || (nextTime && nextTime.getTime() > existingTime.getTime());

      nodeMap.set(nodeId, {
        ...existing,
        url: nodeUrl,
        crawled: existing.crawled || !!normalizedCrawlTime,
        crawlTime: shouldReplace ? normalizedCrawlTime : existing.crawlTime,
        label:
          viewMode === "domain"
            ? nodeUrl
            : shouldReplace
              ? (node.title ?? node.url)
              : existing.label,
        owners: updatedOwners,
        links,
      });
    }

    node.links.forEach((linkObj) => {
      const link = linkObj.url;
      const targetId = resolveId(link);
      const targetUrl = resolveUrl(link);
      const targetExisting = nodeMap.get(targetId);
      const targetOwners = targetExisting?.owners ?? [];
      const updatedTargetOwners = Array.from(new Set([...targetOwners, ...owners]));

      if (!targetExisting) {
        nodeMap.set(targetId, {
          id: targetId,
          url: targetUrl,
          label: viewMode === "domain" ? targetUrl : link,
          crawled: false,
          owners: updatedTargetOwners,
        });
      } else {
        nodeMap.set(targetId, {
          ...targetExisting,
          owners: updatedTargetOwners,
        });
      }

      edgeSet.add(`${nodeId}->${targetId}`);
    });
  });

  return {
    nodes: Array.from(nodeMap.values()),
    edges: Array.from(edgeSet.values()).map((edge) => {
      const [source, target] = edge.split("->");
      return { id: edge, source, target };
    }),
  };
};

export const updateGraphIncrementally = (cy: Core, graphData: GraphData): string[] => {
  if (!cy || !graphData) return [];

  const currentPan = cy.pan();
  const currentZoom = cy.zoom();

  const incomingNodeIds = new Set(graphData.nodes.map((n) => n.id));
  const incomingEdgeIds = new Set(graphData.edges.map((e) => e.id));

  const newNodeIds = graphData.nodes
    .map((n) => n.id)
    .filter((id) => cy.getElementById(id).length === 0);

  // The full layout only runs once per graph, so nodes that show up later have
  // to be placed here rather than piling up at the origin. Stage them off to
  // the side of the existing graph, in a grid so they don't overlap each
  // other; the layout worker then pulls them into place based on their edges
  // while the rest of the graph stays locked where it is.
  const bbox = cy.nodes().length > 0 ? cy.nodes().boundingBox() : { x1: 0, y1: 0, x2: 0, y2: 0 };
  const stagingX = bbox.x2 + NODE_SIZE * 2;
  const columns = Math.max(1, Math.ceil(Math.sqrt(newNodeIds.length)));
  const stagedPositions = new Map(
    newNodeIds.map((id, index) => [
      id,
      {
        x: stagingX + (index % columns) * NODE_SIZE * 1.5,
        y: bbox.y1 + Math.floor(index / columns) * NODE_SIZE * 1.5,
      },
    ]),
  );

  cy.batch(() => {
    // 1. Remove elements that no longer exist (e.g. selection change)
    cy.nodes().forEach((ele) => {
      if (!incomingNodeIds.has(ele.id())) {
        cy.remove(ele);
      }
    });
    cy.edges().forEach((ele) => {
      if (!incomingEdgeIds.has(ele.id())) {
        cy.remove(ele);
      }
    });

    // 2. Add or update nodes
    graphData.nodes.forEach((node) => {
      const ele = cy.getElementById(node.id);
      const label = node.label.length > 16 ? `${node.label.slice(0, 16)}…` : node.label;

      if (ele.length === 0) {
        cy.add({
          group: "nodes",
          data: {
            id: node.id,
            label,
            crawled: node.crawled,
            ownerId: node.owners[0],
          },
          position: stagedPositions.get(node.id),
          classes: PENDING_LAYOUT_CLASS,
        });
      } else {
        ele.data({
          label,
          crawled: node.crawled,
          ownerId: node.owners[0],
        });
      }
    });

    // 3. Add edges
    graphData.edges.forEach((edge) => {
      const ele = cy.getElementById(edge.id);
      if (ele.length === 0) {
        // Ensure both endpoints exist in cytoscape before adding edge
        if (
          cy.getElementById(edge.source).length > 0 &&
          cy.getElementById(edge.target).length > 0
        ) {
          cy.add({
            group: "edges",
            data: {
              id: edge.id,
              source: edge.source,
              target: edge.target,
            },
          });
        }
      }
    });
  });

  // 4. Restore viewport
  cy.viewport({
    zoom: currentZoom,
    pan: currentPan,
  });

  return newNodeIds;
};
