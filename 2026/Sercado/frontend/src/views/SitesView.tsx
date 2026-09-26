import {
  ActionIcon,
  Badge,
  Button,
  Group,
  MultiSelect,
  Pagination,
  Paper,
  Select,
  SimpleGrid,
  Stack,
  Switch,
  Table,
  Text,
  TextInput,
  Title,
  Tooltip,
} from "@mantine/core";
import {
  IconPencil,
  IconPlayerPlay,
  IconPlayerPause,
  IconPlus,
  IconSearch,
  IconTrash,
  IconReload,
} from "@tabler/icons-react";
import { useDisclosure } from "@mantine/hooks";
import { useMemo, useState } from "react";
import ConfirmDeleteModal from "../components/ConfirmDeleteModal";
import WebsiteFormModal from "../components/WebsiteFormModal";
import { PAGE_SIZE, STATUS_COLORS } from "../constants";
import { useExecutions } from "../context/ExecutionsContext";
import { useWebsites } from "../context/WebsitesContext";
import type { WebsiteFormValues, WebsiteRecord } from "../types";
import { formatDateTime } from "../utils/date";
import { getExecutionTime } from "../utils/execution";

const SitesView = () => {
  const { websites, tagOptions, createWebsite, updateWebsite, deleteWebsite } = useWebsites();
  const { latestExecutions, startExecution, pauseExecution } = useExecutions();
  const [websiteModalOpened, websiteModalHandlers] = useDisclosure(false);
  const [editingWebsite, setEditingWebsite] = useState<WebsiteRecord | null>(null);
  const [deleteTarget, setDeleteTarget] = useState<WebsiteRecord | null>(null);
  const [siteFilterUrl, setSiteFilterUrl] = useState("");
  const [siteFilterLabel, setSiteFilterLabel] = useState("");
  const [siteFilterTags, setSiteFilterTags] = useState<string[]>([]);
  const [siteSort, setSiteSort] = useState("url-asc");
  const [sitePage, setSitePage] = useState(1);

  const filteredSites = useMemo(() => {
    const urlQuery = siteFilterUrl.toLowerCase();
    const labelQuery = siteFilterLabel.toLowerCase();
    return websites
      .filter((site) => {
        if (urlQuery && !site.url.toLowerCase().includes(urlQuery)) {
          return false;
        }
        if (labelQuery && !site.label.toLowerCase().includes(labelQuery)) {
          return false;
        }
        if (siteFilterTags.length > 0) {
          return siteFilterTags.every((tag) => site.tags.includes(tag));
        }
        return true;
      })
      .sort((a, b) => {
        const latestA = latestExecutions.get(a.id);
        const latestB = latestExecutions.get(b.id);
        if (siteSort === "url-asc") {
          return a.url.localeCompare(b.url);
        }
        if (siteSort === "url-desc") {
          return b.url.localeCompare(a.url);
        }
        const timeA = getExecutionTime(latestA);
        const timeB = getExecutionTime(latestB);
        return siteSort === "last-desc" ? timeB - timeA : timeA - timeB;
      });
  }, [websites, siteFilterUrl, siteFilterLabel, siteFilterTags, siteSort, latestExecutions]);

  const openCreateWebsite = () => {
    setEditingWebsite(null);
    websiteModalHandlers.open();
  };

  const openEditWebsite = (record: WebsiteRecord) => {
    setEditingWebsite(record);
    websiteModalHandlers.open();
  };

  const handleSaveWebsite = async (values: WebsiteFormValues) => {
    if (editingWebsite) {
      await updateWebsite(editingWebsite.id, values);
      setEditingWebsite(null);
      return;
    }
    await createWebsite(values);
  };

  const handleDeleteWebsite = () => {
    if (!deleteTarget) {
      return;
    }
    deleteWebsite(deleteTarget.id);
    setDeleteTarget(null);
  };

  const siteTotalPages = Math.max(1, Math.ceil(filteredSites.length / PAGE_SIZE));
  const sitePageItems = filteredSites.slice((sitePage - 1) * PAGE_SIZE, sitePage * PAGE_SIZE);

  return (
    <Stack>
      <Group justify="space-between">
        <Title order={2}>Website records</Title>
        <Button leftSection={<IconPlus size={16} />} onClick={openCreateWebsite}>
          New website
        </Button>
      </Group>

      <Paper withBorder p="md">
        <SimpleGrid cols={{ base: 1, sm: 2, lg: 4 }}>
          <TextInput
            label="Filter by URL"
            placeholder="https://"
            leftSection={<IconSearch size={14} />}
            value={siteFilterUrl}
            onChange={(event) => {
              setSiteFilterUrl(event.currentTarget.value);
              setSitePage(1);
            }}
          />
          <TextInput
            label="Filter by label"
            placeholder="Marketing"
            leftSection={<IconSearch size={14} />}
            value={siteFilterLabel}
            onChange={(event) => {
              setSiteFilterLabel(event.currentTarget.value);
              setSitePage(1);
            }}
          />
          <MultiSelect
            label="Filter by tags"
            data={tagOptions}
            placeholder="Select tags"
            value={siteFilterTags}
            onChange={(value) => {
              setSiteFilterTags(value);
              setSitePage(1);
            }}
            searchable
            clearable
          />
          <Select
            label="Sort by"
            value={siteSort}
            onChange={(value) => {
              setSiteSort(value ?? "url-asc");
              setSitePage(1);
            }}
            data={[
              { value: "url-asc", label: "URL (A → Z)" },
              { value: "url-desc", label: "URL (Z → A)" },
              { value: "last-desc", label: "Last execution (Newest)" },
              { value: "last-asc", label: "Last execution (Oldest)" },
            ]}
          />
        </SimpleGrid>
      </Paper>

      <Paper withBorder p="md">
        <Text size="sm" c="dimmed" mb="sm">
          {filteredSites.length} website records
        </Text>
        <Table highlightOnHover striped>
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Label</Table.Th>
              <Table.Th>URL</Table.Th>
              <Table.Th>Periodicity</Table.Th>
              <Table.Th>Tags</Table.Th>
              <Table.Th>Last execution</Table.Th>
              <Table.Th>Status</Table.Th>
              <Table.Th>Active</Table.Th>
              <Table.Th>Actions</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>
            {sitePageItems.map((site) => {
              const latest = latestExecutions.get(site.id);
              const status = latest?.status ?? "queued";
              return (
                <Table.Tr key={site.id}>
                  <Table.Td>{site.label}</Table.Td>
                  <Table.Td>{site.url}</Table.Td>
                  <Table.Td>
                    <Badge variant="light">{site.periodicity}</Badge>
                  </Table.Td>
                  <Table.Td>
                    <Group gap={6}>
                      {site.tags.length === 0 && (
                        <Text size="sm" c="dimmed">
                          —
                        </Text>
                      )}
                      {site.tags.map((tag) => (
                        <Badge key={tag} variant="outline">
                          {tag}
                        </Badge>
                      ))}
                    </Group>
                  </Table.Td>
                  <Table.Td>
                    {latest?.startTime ? formatDateTime(latest.startTime) : "Not yet started"}
                  </Table.Td>
                  <Table.Td>
                    <Badge color={STATUS_COLORS[status]}>{status}</Badge>
                  </Table.Td>
                  <Table.Td>
                    <Switch
                      checked={site.active}
                      onChange={(event) =>
                        updateWebsite(site.id, { active: event.currentTarget.checked })
                      }
                    />
                  </Table.Td>
                  <Table.Td>
                    <Group gap={8}>
                      {status === "queued" ? (
                        <Tooltip label="Starting execution" withArrow>
                          <ActionIcon
                            variant="light"
                            aria-label="Starting execution"
                            loading
                            disabled
                          >
                            <IconPlayerPlay size={16} />
                          </ActionIcon>
                        </Tooltip>
                      ) : status === "running" ? (
                        <Tooltip label="Pause execution" withArrow>
                          <ActionIcon
                            variant="light"
                            color="yellow"
                            aria-label="Pause execution"
                            onClick={() => pauseExecution(site.id)}
                          >
                            <IconPlayerPause size={16} />
                          </ActionIcon>
                        </Tooltip>
                      ) : status === "paused" ? (
                        <Tooltip label="Resume execution" withArrow>
                          <ActionIcon
                            variant="light"
                            aria-label="Resume execution"
                            onClick={() => startExecution(site.id)}
                          >
                            <IconPlayerPlay size={16} />
                          </ActionIcon>
                        </Tooltip>
                      ) : (
                        <Tooltip label="Rerun Scraping" withArrow>
                          <ActionIcon
                            variant="light"
                            aria-label="Rerun Scraping"
                            onClick={() => startExecution(site.id)}
                          >
                            <IconReload size={16} />
                          </ActionIcon>
                        </Tooltip>
                      )}
                      <Tooltip label="Edit website" withArrow>
                        <ActionIcon
                          variant="light"
                          aria-label="Edit website"
                          onClick={() => openEditWebsite(site)}
                        >
                          <IconPencil size={16} />
                        </ActionIcon>
                      </Tooltip>
                      <Tooltip label="Delete website" withArrow>
                        <ActionIcon
                          variant="light"
                          color="red"
                          aria-label="Delete website"
                          onClick={() => setDeleteTarget(site)}
                        >
                          <IconTrash size={16} />
                        </ActionIcon>
                      </Tooltip>
                    </Group>
                  </Table.Td>
                </Table.Tr>
              );
            })}
          </Table.Tbody>
        </Table>
        <Group justify="space-between" mt="md">
          <Pagination value={sitePage} onChange={setSitePage} total={siteTotalPages} />
          <Text size="sm" c="dimmed">
            Page {sitePage} of {siteTotalPages}
          </Text>
        </Group>
      </Paper>

      <WebsiteFormModal
        opened={websiteModalOpened}
        onClose={websiteModalHandlers.close}
        onSubmit={handleSaveWebsite}
        initialValues={editingWebsite ?? {}}
        title={editingWebsite ? "Edit website record" : "Create website record"}
        submitLabel={editingWebsite ? "Save changes" : "Create website"}
      />

      <ConfirmDeleteModal
        opened={deleteTarget !== null}
        label={deleteTarget?.label ?? "this record"}
        onCancel={() => setDeleteTarget(null)}
        onConfirm={handleDeleteWebsite}
      />
    </Stack>
  );
};

export default SitesView;
