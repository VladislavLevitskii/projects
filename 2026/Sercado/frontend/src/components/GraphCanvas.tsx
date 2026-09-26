// GraphCanvas.tsx

import { Box, Loader, Paper, Text, useMantineTheme } from "@mantine/core";
import cytoscape from "cytoscape";
import { useEffect, useMemo, useRef, useState } from "react";
import type { GraphData, GraphNode } from "../types";
import { NODE_SIZE, PENDING_LAYOUT_CLASS, updateGraphIncrementally } from "../utils/graph";
import { getWebsiteColor } from "../utils/colors";

const createLayoutWorker = () =>
  new Worker(new URL("./graphLayout.worker.ts", import.meta.url), {
    type: "module",
  });

const MAX_VERTICES = 200;

const filterToCrawled = (data: GraphData, viewMode: "website" | "domain"): GraphData => {
  if (viewMode === "domain" || data.nodes.length <= MAX_VERTICES) {
    return data;
  }

  const crawledNodes = data.nodes.filter((node) => node.crawled);
  const crawledIds = new Set(crawledNodes.map((node) => node.id));
  const crawledEdges = data.edges.filter(
    (edge) => crawledIds.has(edge.source) && crawledIds.has(edge.target),
  );
  return { nodes: crawledNodes, edges: crawledEdges };
};

const GraphCanvas = ({
  data,
  graphKey,
  onNodeDoubleClick,
  viewMode,
}: {
  data: GraphData;
  graphKey: string;
  onNodeDoubleClick: (node: GraphNode) => void;
  viewMode: "website" | "domain";
}) => {
  const theme = useMantineTheme();
  const containerRef = useRef<HTMLDivElement | null>(null);
  const cyRef = useRef<cytoscape.Core | null>(null);
  const workerRef = useRef<Worker | null>(null);
  const isInitialLayoutRef = useRef(true);
  const isLayoutRunningRef = useRef(false);
  const graphKeyRef = useRef(graphKey);
  const viewModeRef = useRef(viewMode);
  const [isLayoutLoading, setIsLayoutLoading] = useState(false);

  const [tooltip, setTooltip] = useState({
    show: false,
    text: "",
    x: 0,
    y: 0,
  });

  const renderData = useMemo(() => filterToCrawled(data, viewMode), [data, viewMode]);

  const nodeMap = useMemo(
    () => new Map(renderData.nodes.map((n) => [n.id, n])),
    [renderData.nodes],
  );

  const layoutIterations = useMemo(() => {
    // const totalElements = renderData.nodes.length + renderData.edges.length;

    const estimatedCalculationLength = renderData.nodes.length ** 2;

    return Math.max(10, Math.ceil(5e7 / estimatedCalculationLength));
  }, [renderData.edges.length, renderData.nodes.length]);

  // Create worker once.
  useEffect(() => {
    workerRef.current = createLayoutWorker();

    // The lock means "a job is in flight on the current worker", so a fresh
    // worker has to clear it: whatever job it replaces was terminated and its
    // onmessage will never fire. Without this, StrictMode's remount in dev
    // leaves the lock set forever and the layout never starts.
    if (isLayoutRunningRef.current) {
      isLayoutRunningRef.current = false;
      setIsLayoutLoading(false);
    }

    return () => {
      workerRef.current?.terminate();
      workerRef.current = null;
    };
  }, []);

  // A different graph (website selection / view mode) invalidates any layout
  // still being computed. The lock only exists to keep upstream updates from
  // piling up jobs for the *same* graph, so release it here, abandon the
  // in-flight job, and let the layout effect below start a fresh one.
  useEffect(() => {
    if (graphKeyRef.current === graphKey) {
      return;
    }

    graphKeyRef.current = graphKey;

    // Switching view mode changes every node's identity (URL vs domain), so
    // the canvas has to be rebuilt from scratch. Just adding/removing a
    // website to the existing selection shouldn't: the nodes that survive
    // keep their ids, so let the incremental branch below fold the change in
    // around them instead of blowing away the whole layout every time.
    const viewModeChanged = viewModeRef.current !== viewMode;
    viewModeRef.current = viewMode;

    // A running worker can only be stopped by terminating it.
    workerRef.current?.terminate();
    workerRef.current = createLayoutWorker();

    isLayoutRunningRef.current = false;
    isInitialLayoutRef.current =
      viewModeChanged || !cyRef.current || cyRef.current.nodes().length === 0;
    setIsLayoutLoading(false);
  }, [graphKey, viewMode]);

  // Register double-click handler with fresh nodeMap reference
  useEffect(() => {
    const cy = cyRef.current;
    if (!cy) return;

    const handler = (event: cytoscape.EventObject) => {
      const node = nodeMap.get(event.target.id());
      if (node) {
        onNodeDoubleClick(node);
      }
    };

    cy.off("dblclick", "node");
    cy.on("dblclick", "node", handler);

    return () => {
      cy.off("dblclick", "node", handler);
    };
  }, [nodeMap, onNodeDoubleClick]);

  // Register hover handler with fresh nodeMap reference. cy is only created
  // once, so a handler bound at creation time would keep the nodeMap from
  // that moment and never see nodes added afterwards (e.g. from adding
  // another website to the selection) - rebind it here instead, same as the
  // dblclick handler above.
  useEffect(() => {
    const cy = cyRef.current;
    if (!cy) return;

    const handler = (event: cytoscape.EventObject) => {
      const node = nodeMap.get(event.target.id());

      if (node) {
        setTooltip({
          show: true,
          text: node.url,
          x: event.renderedPosition.x,
          y: event.renderedPosition.y,
        });
      }

      const connectedEdges = event.target.connectedEdges();

      connectedEdges.addClass("highlighted");
      event.target.cy().edges().not(connectedEdges).addClass("dimmed");
    };

    cy.off("mouseover", "node");
    cy.on("mouseover", "node", handler);

    return () => {
      cy.off("mouseover", "node", handler);
    };
  }, [nodeMap]);

  useEffect(() => {
    if (!containerRef.current) {
      return;
    }

    if (!cyRef.current) {
      cyRef.current = cytoscape({
        container: containerRef.current,
        elements: [],
        wheelSensitivity: 5,
        style: [
          {
            selector: "node",
            style: {
              width: NODE_SIZE,
              height: NODE_SIZE,
              shape: "ellipse",
              "background-color": (ele: cytoscape.NodeSingular) =>
                getWebsiteColor(ele.data("ownerId")),
              "background-opacity": (ele: cytoscape.NodeSingular) =>
                ele.data("crawled") ? 1 : 0.35,
              label: "data(label)",
              color: "#fff",
              "text-outline-width": 0.8,
              "text-outline-color": "rgba(0, 0, 0, 0.55)",
              "font-size": 10,
              "text-valign": "center",
              "text-halign": "center",
              "text-wrap": "wrap",
              "text-max-width": `${NODE_SIZE - 10}px`,
            },
          },
          {
            selector: "edge",
            style: {
              width: 1.2,
              "line-color": theme.colors.gray[4],
              "target-arrow-color": theme.colors.gray[5],
              "target-arrow-shape": "triangle",
              "curve-style": "bezier",
            },
          },
          {
            selector: "edge.highlighted",
            style: {
              width: 2.5,
              "line-color": theme.colors.blue[6],
              "target-arrow-color": theme.colors.blue[6],
              "z-index": 999,
            },
          },
          {
            selector: "edge.dimmed",
            style: {
              opacity: 0.15,
            },
          },
          {
            selector: `node.${PENDING_LAYOUT_CLASS}`,
            style: {
              display: "none",
            },
          },
        ],
      });

      cyRef.current.on("mousemove", "node", (event) => {
        setTooltip((prev) => ({
          ...prev,
          x: event.renderedPosition.x,
          y: event.renderedPosition.y,
        }));
      });

      cyRef.current.on("mouseout", "node", (event) => {
        setTooltip((prev) => ({ ...prev, show: false }));
        event.target.cy().edges().removeClass("highlighted dimmed");
      });
    }

    const cy = cyRef.current;

    const newNodeIds = updateGraphIncrementally(cy, renderData);

    if (isInitialLayoutRef.current && renderData.nodes.length > 0 && !isLayoutRunningRef.current) {
      // The full layout runs once per graph, with a loading overlay since
      // nothing is on screen to look at yet.
      const worker = workerRef.current;

      if (!worker) {
        return;
      }

      isLayoutRunningRef.current = true;
      setIsLayoutLoading(true);

      worker.onmessage = (event) => {
        for (const { id, position } of event.data) {
          cy.$id(id).position(position);
        }

        cy.nodes().removeClass(PENDING_LAYOUT_CLASS);
        cy.fit(undefined, 30);
        isInitialLayoutRef.current = false;
        isLayoutRunningRef.current = false;
        setIsLayoutLoading(false);
      };

      worker.postMessage({
        nodes: renderData.nodes.map((node) => ({
          data: { id: node.id },
        })),
        edges: renderData.edges.map((edge) => ({
          data: edge,
        })),
        layoutIterations,
      });
    } else if (
      !isInitialLayoutRef.current &&
      newNodeIds.length > 0 &&
      !isLayoutRunningRef.current
    ) {
      // New nodes showed up after the initial layout. updateGraphIncrementally
      // already staged them to the side; re-run the layout worker starting
      // from the current positions (nothing locked) so the whole graph can
      // reflow around the newcomers, then apply every returned position, not
      // just the new nodes'. Runs quietly in the background - no loading
      // overlay, since the graph stays interactive while it settles.
      const worker = workerRef.current;

      if (!worker) {
        return;
      }

      isLayoutRunningRef.current = true;

      worker.onmessage = (event) => {
        for (const { id, position } of event.data) {
          cy.$id(id).position(position);
        }

        cy.nodes().removeClass(PENDING_LAYOUT_CLASS);
        isLayoutRunningRef.current = false;
      };

      worker.postMessage({
        nodes: cy.nodes().map((node) => ({
          data: { id: node.id() },
          position: node.position(),
        })),
        edges: cy.edges().map((edge) => ({
          data: {
            id: edge.id(),
            source: edge.source().id(),
            target: edge.target().id(),
          },
        })),
        layoutIterations,
        incremental: true,
      });
    }
  }, [renderData, graphKey, theme, nodeMap, onNodeDoubleClick, layoutIterations]);

  return (
    <div style={{ position: "relative", width: "100%" }}>
      <Box
        ref={containerRef}
        h="70vh"
        style={{
          width: "100%",
          borderRadius: 8,
          overflow: "hidden",
        }}
      />

      {isLayoutLoading && (
        <Box
          style={{
            position: "absolute",
            inset: 0,
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
            backgroundColor: "var(--mantine-color-body)",
            borderRadius: 8,
            zIndex: 10,
          }}
        >
          <Loader size="lg" />
        </Box>
      )}

      {tooltip.show && (
        <Paper
          withBorder
          shadow="sm"
          p="xs"
          style={{
            position: "absolute",
            top: tooltip.y + 15,
            left: tooltip.x + 15,
            zIndex: 10,
            pointerEvents: "none",
            maxWidth: 350,
          }}
        >
          <Text size="xs" style={{ wordBreak: "break-all" }}>
            {tooltip.text}
          </Text>
        </Paper>
      )}
    </div>
  );
};

export default GraphCanvas;
