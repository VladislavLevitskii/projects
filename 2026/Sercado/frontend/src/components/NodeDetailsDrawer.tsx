import { Anchor, Badge, Button, Divider, Drawer, Group, Select, Stack, Text } from "@mantine/core";
import { IconPlayerPlay, IconPlus } from "@tabler/icons-react";
import { useExecutions } from "../context/ExecutionsContext";
import { useWebsites } from "../context/WebsitesContext";
import type { GraphNode } from "../types";
import { formatDateTime } from "../utils/date";

const NodeDetailsDrawer = ({
  opened,
  onClose,
  node,
  selectedOwnerId,
  onOwnerChange,
  onCreateWebsite,
}: {
  opened: boolean;
  onClose: () => void;
  node: GraphNode | null;
  selectedOwnerId: string | null;
  onOwnerChange: (value: string | null) => void;
  onCreateWebsite: (url: string) => void;
}) => {
  const { websiteMap } = useWebsites();
  const { startExecution } = useExecutions();
  const ownerBadges =
    node?.owners
      .map((id) => websiteMap.get(id))
      .filter((owner): owner is NonNullable<typeof owner> => Boolean(owner)) ?? [];

  return (
    <Drawer opened={opened} onClose={onClose} position="right" title="Node details" size="md">
      {node ? (
        <Stack>
          <Text size="sm" c="dimmed">
            URL
          </Text>
          <Anchor
            href={node.url}
            target="_blank"
            rel="noopener noreferrer"
            style={{ wordBreak: "break-all" }}
          >
            {node.url}
          </Anchor>
          <Divider />
          {node.crawled ? (
            <Stack style={{ height: "100%", display: "flex", flexDirection: "column" }}>
              <div style={{ flex: 1 }}>
                <Stack>
                  <Text size="sm" c="dimmed">
                    Crawl time
                  </Text>
                  <Text>{formatDateTime(node.crawlTime)}</Text>
                  <Text size="sm" c="dimmed">
                    Website records
                  </Text>
                  <Group gap="xs">
                    {ownerBadges.map((owner) => (
                      <Badge key={owner.id} variant="light">
                        {owner.label}
                      </Badge>
                    ))}
                  </Group>
                  <Divider />
                  <Select
                    label="Start execution for"
                    data={ownerBadges.map((owner) => ({
                      value: owner.id,
                      label: owner.label,
                    }))}
                    value={selectedOwnerId ?? undefined}
                    onChange={(value) => onOwnerChange(value ?? null)}
                  />
                  <Button
                    leftSection={<IconPlayerPlay size={16} />}
                    disabled={!selectedOwnerId}
                    onClick={() => selectedOwnerId && startExecution(selectedOwnerId)}
                  >
                    Start execution
                  </Button>
                </Stack>
              </div>
              {node.links && node.links.length > 0 && (
                <Stack
                  gap="xs"
                  style={{ marginTop: "auto", paddingTop: "var(--mantine-spacing-md)" }}
                >
                  <Divider />
                  <Text size="sm" c="dimmed">
                    Outgoing links
                  </Text>
                  <Stack gap="xs">
                    {node.links.map((link, index) => (
                      <Anchor
                        key={index}
                        href={link}
                        target="_blank"
                        rel="noopener noreferrer"
                        size="sm"
                        style={{ wordBreak: "break-all" }}
                      >
                        {link}
                      </Anchor>
                    ))}
                  </Stack>
                </Stack>
              )}
            </Stack>
          ) : (
            <Stack>
              <Text size="sm" c="dimmed">
                This node is outside the current boundary RegExp.
              </Text>
              <Button
                leftSection={<IconPlus size={16} />}
                onClick={() => onCreateWebsite(node.url)}
              >
                Create website record
              </Button>
            </Stack>
          )}
        </Stack>
      ) : (
        <Text size="sm" c="dimmed">
          Select a node to see more details.
        </Text>
      )}
    </Drawer>
  );
};

export default NodeDetailsDrawer;
