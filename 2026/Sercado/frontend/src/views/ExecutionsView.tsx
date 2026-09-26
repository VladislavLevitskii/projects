import {
  Badge,
  Button,
  Group,
  Pagination,
  Paper,
  Select,
  Stack,
  Table,
  Text,
  Title,
} from "@mantine/core";
import { IconPlayerPlay } from "@tabler/icons-react";
import { useMemo, useState } from "react";
import { PAGE_SIZE, STATUS_COLORS } from "../constants";
import { useExecutions } from "../context/ExecutionsContext";
import { useWebsites } from "../context/WebsitesContext";
import { formatDateTime } from "../utils/date";

const ExecutionsView = () => {
  const { websiteMap, websiteOptions } = useWebsites();
  const { executions, startExecution } = useExecutions();
  const [executionFilterSite, setExecutionFilterSite] = useState<string | null>(null);
  const [executionPage, setExecutionPage] = useState(1);

  const filteredExecutions = useMemo(() => {
    return executions.filter((exec) =>
      executionFilterSite ? exec.websiteId === executionFilterSite : true,
    );
  }, [executions, executionFilterSite]);

  const executionTotalPages = Math.max(1, Math.ceil(filteredExecutions.length / PAGE_SIZE));
  const executionPageItems = filteredExecutions.slice(
    (executionPage - 1) * PAGE_SIZE,
    executionPage * PAGE_SIZE,
  );

  return (
    <Stack>
      <Group justify="space-between">
        <Title order={2}>Executions</Title>
        <Group>
          <Select
            placeholder="Filter by website"
            data={websiteOptions}
            value={executionFilterSite ?? undefined}
            onChange={(value) => {
              setExecutionFilterSite(value ?? null);
              setExecutionPage(1);
            }}
            allowDeselect
          />
          <Button
            leftSection={<IconPlayerPlay size={16} />}
            disabled={!executionFilterSite}
            onClick={() => executionFilterSite && startExecution(executionFilterSite)}
          >
            Start execution
          </Button>
        </Group>
      </Group>
      <Paper withBorder p="md">
        <Table highlightOnHover striped>
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Website</Table.Th>
              <Table.Th>Status</Table.Th>
              <Table.Th>Start</Table.Th>
              <Table.Th>End</Table.Th>
              <Table.Th>Pages crawled</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>
            {executionPageItems.map((execution) => (
              <Table.Tr key={execution.id}>
                <Table.Td>{websiteMap.get(execution.websiteId)?.label ?? "Unknown"}</Table.Td>
                <Table.Td>
                  <Badge color={STATUS_COLORS[execution.status]}>{execution.status}</Badge>
                </Table.Td>
                <Table.Td>{formatDateTime(execution.startTime)}</Table.Td>
                <Table.Td>{formatDateTime(execution.endTime)}</Table.Td>
                <Table.Td>{execution.pagesCrawled}</Table.Td>
              </Table.Tr>
            ))}
          </Table.Tbody>
        </Table>
        <Group justify="space-between" mt="md">
          <Pagination
            value={executionPage}
            onChange={setExecutionPage}
            total={executionTotalPages}
          />
          <Text size="sm" c="dimmed">
            Page {executionPage} of {executionTotalPages}
          </Text>
        </Group>
      </Paper>
    </Stack>
  );
};

export default ExecutionsView;
