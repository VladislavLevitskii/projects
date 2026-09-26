import { Button, Group, Modal, Stack, Text } from "@mantine/core";

const ConfirmDeleteModal = ({
  opened,
  label,
  onCancel,
  onConfirm,
}: {
  opened: boolean;
  label: string;
  onCancel: () => void;
  onConfirm: () => void;
}) => (
  <Modal opened={opened} onClose={onCancel} title="Delete website record" centered>
    <Stack>
      <Text>
        Delete <strong>{label}</strong> and all related executions? This cannot be undone.
      </Text>
      <Group justify="flex-end">
        <Button variant="default" onClick={onCancel}>
          Cancel
        </Button>
        <Button color="red" onClick={onConfirm}>
          Delete
        </Button>
      </Group>
    </Stack>
  </Modal>
);

export default ConfirmDeleteModal;
