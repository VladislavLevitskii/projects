import cytoscape from "cytoscape";
import { NODE_SIZE } from "../utils/graph";
import type { GraphLayoutWorkerMessage } from "../types";

self.onmessage = (event: MessageEvent<GraphLayoutWorkerMessage>) => {
  const { nodes, edges, layoutIterations } = event.data;

  console.log("nodes:", nodes);
  console.log("edges:", edges);
  console.log("iterations:", layoutIterations);

  const cy = cytoscape({
    headless: true,
    styleEnabled: true,
    elements: {
      nodes,
      edges,
    },
    style: [
      {
        selector: "node",
        style: {
          width: NODE_SIZE,
          height: NODE_SIZE,
        },
      },
    ],
  });

  cy.layout({
    name: "cose",
    animate: false,
    randomize: false,
    nodeRepulsion: 1e6,
    gravity: 1,
    numIter: layoutIterations,
    nodeOverlap: 0,
    idealEdgeLength: 1000,
    avoidOverlap: false,
  }).run();

  cy.layout({
    name: "cose",
    animate: false,
    randomize: false,
    nodeRepulsion: 3e6,
    gravity: 1,
    numIter: layoutIterations,
    nodeOverlap: edges.length / 10 + 40,
    // idealEdgeLength: 500,
    avoidOverlap: true,
  }).run();

  self.postMessage(
    cy.nodes().map((node) => ({
      id: node.id(),
      position: node.position(),
    })),
  );
};
