import {
  Button,
  ColorSwatch,
  Group,
  MultiSelect,
  Paper,
  SegmentedControl,
  SimpleGrid,
  Stack,
  Text,
  Title,
} from "@mantine/core";
import { IconListDetails, IconRefresh, IconAlertTriangle } from "@tabler/icons-react";
import { useDisclosure } from "@mantine/hooks";
import { useExecutions } from "../context/ExecutionsContext";
import { useGraph } from "../context/GraphContext";
import { useWebsites } from "../context/WebsitesContext";
import GraphCanvas from "../components/GraphCanvas";
import NodeDetailsDrawer from "../components/NodeDetailsDrawer";
import WebsiteFormModal from "../components/WebsiteFormModal";
import type { GraphNode, WebsiteFormValues } from "../types";
import { MAX_GRAPH_NODES } from "../constants";
import { getWebsiteColor } from "../utils/colors";
import { getDomain } from "../utils/url";
import { useState } from "react";

const VisualizationView = () => {
  const { createWebsite, websiteOptions } = useWebsites();
  const { latestExecutions, refreshExecutions } = useExecutions();
  const {
    resolvedSelectedWebsiteIds,
    setSelectedWebsiteIds,
    viewMode,
    setViewMode,
    liveMode,
    setLiveMode,
    graphData,
    refreshGraph,
  } = useGraph();

  const [isRefreshing, setIsRefreshing] = useState(false);
  const [nodeDrawerOpened, nodeDrawerHandlers] = useDisclosure(false);
  const [selectedNode, setSelectedNode] = useState<GraphNode | null>(null);
  const [selectedOwnerId, setSelectedOwnerId] = useState<string | null>(null);
  const [createPrefill, setCreatePrefill] = useState<Partial<WebsiteFormValues> | null>(null);

  const handleRefresh = async () => {
    setIsRefreshing(true);
    try {
      await Promise.all([refreshGraph(), refreshExecutions()]);
    } finally {
      setIsRefreshing(false);
    }
  };

  const handleNodeDoubleClick = (node: GraphNode) => {
    setSelectedNode(node);
    setSelectedOwnerId(node.owners[0] ?? null);
    nodeDrawerHandlers.open();
  };

  const handleCreateWebsite = async (values: WebsiteFormValues) => {
    const newWebsite = await createWebsite(values);
    if (newWebsite?.id) {
      setSelectedWebsiteIds((prev) => Array.from(new Set([newWebsite.id, ...prev])));
      setLiveMode("live");
    }
  };

  const totalCrawled =
    viewMode === "domain"
      ? graphData.nodes.length
      : resolvedSelectedWebsiteIds.reduce((sum, id) => {
          return sum + (latestExecutions.get(id)?.pagesCrawled ?? 0);
        }, 0);

  return (
    <Stack>
      <Group justify="space-between">
        <Title order={2}>Crawl map</Title>
        <Group>
          <SegmentedControl
            value={viewMode}
            onChange={(value) => setViewMode(value as "website" | "domain")}
            data={[
              { label: "Website view", value: "website" },
              { label: "Domain view", value: "domain" },
            ]}
          />
          <SegmentedControl
            value={liveMode}
            onChange={(value) => setLiveMode(value as "live" | "static")}
            data={[
              { label: "Live", value: "live" },
              { label: "Static", value: "static" },
            ]}
          />
          {liveMode === "static" && (
            <Button
              leftSection={<IconRefresh size={16} />}
              variant="default"
              loading={isRefreshing}
              onClick={handleRefresh}
            >
              Refresh
            </Button>
          )}
        </Group>
      </Group>

      <Paper withBorder p="md">
        <SimpleGrid cols={{ base: 1, md: 2 }}>
          <MultiSelect
            label="Active selection"
            data={websiteOptions}
            value={resolvedSelectedWebsiteIds}
            onChange={setSelectedWebsiteIds}
            searchable
            clearable
            placeholder="Select websites"
          />
          <Stack gap="xs">
            <Text size="sm" c="dimmed">
              Double-click a node to inspect details, start a new execution, or add the site to the
              active selection. Each website has its own color; faded nodes are outside the crawl
              boundary.
            </Text>
            <Group gap="md" wrap="wrap">
              {resolvedSelectedWebsiteIds.map((id) => (
                <Group gap={6} key={id}>
                  <ColorSwatch color={getWebsiteColor(id)} size={14} />
                  <Text size="sm">
                    {websiteOptions.find((option) => option.value === id)?.label ?? id}
                  </Text>
                </Group>
              ))}
            </Group>
          </Stack>
        </SimpleGrid>
      </Paper>

      <Paper withBorder p="md">
        {totalCrawled > MAX_GRAPH_NODES ? (
          <Stack align="center" py="xl">
            <IconAlertTriangle size={40} color="var(--mantine-color-yellow-6)" />
            <Text size="md" fw={500}>
              I'm not gonna show this 🙏
            </Text>
            <Text size="sm" c="dimmed">
              This selection contains {totalCrawled} crawled nodes. Rendering more than{" "}
              {MAX_GRAPH_NODES} nodes may cause performance issues or crash your browser.
            </Text>
          </Stack>
        ) : graphData.nodes.length === 0 ? (
          <Stack align="center" py="xl">
            <IconListDetails size={40} />
            <Text size="sm" c="dimmed">
              No crawl data yet. Start an execution to populate the graph.
            </Text>
          </Stack>
        ) : (
          <GraphCanvas
            graphKey={`${viewMode}:${[...resolvedSelectedWebsiteIds].sort().join(",")}`}
            data={graphData}
            onNodeDoubleClick={handleNodeDoubleClick}
            viewMode={viewMode}
          />
        )}
      </Paper>

      <NodeDetailsDrawer
        opened={nodeDrawerOpened}
        onClose={() => {
          nodeDrawerHandlers.close();
          setSelectedNode(null);
        }}
        node={selectedNode}
        selectedOwnerId={selectedOwnerId}
        onOwnerChange={setSelectedOwnerId}
        onCreateWebsite={(url) => setCreatePrefill({ url, label: getDomain(url) })}
      />

      <WebsiteFormModal
        opened={createPrefill !== null}
        onClose={() => setCreatePrefill(null)}
        onSubmit={handleCreateWebsite}
        initialValues={createPrefill ?? {}}
        title="Create website record"
        submitLabel="Create website"
      />
    </Stack>
  );
};

export default VisualizationView;
