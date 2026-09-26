import { Button, Group, Modal, Select, Stack, Switch, TagsInput, TextInput } from "@mantine/core";
import { useForm } from "@mantine/form";
import { useEffect } from "react";
import { PERIODICITY_OPTIONS } from "../constants";
import type { WebsiteFormValues } from "../types";
import { buildBoundaryRegexp } from "../utils/url";

const withDefaults = (values: Partial<WebsiteFormValues>): WebsiteFormValues => ({
  url: values.url ?? "",
  boundaryRegexp: values.boundaryRegexp ?? buildBoundaryRegexp(values.url),
  periodicity: values.periodicity ?? "hour",
  label: values.label ?? "",
  active: values.active ?? true,
  tags: values.tags ?? [],
});

const WebsiteFormModal = ({
  opened,
  onClose,
  onSubmit,
  initialValues,
  title,
  submitLabel,
}: {
  opened: boolean;
  onClose: () => void;
  onSubmit: (values: WebsiteFormValues) => void;
  initialValues: Partial<WebsiteFormValues>;
  title: string;
  submitLabel: string;
}) => {
  const form = useForm<WebsiteFormValues>({
    initialValues: withDefaults(initialValues),
    validate: {
      url: (value) => (!value.trim() ? "URL is required" : null),
      boundaryRegexp: (value) => {
        if (!value.trim()) {
          return "Boundary RegExp is required";
        }
        try {
          new RegExp(value);
          return null;
        } catch {
          return "Invalid regular expression";
        }
      },
      label: (value) => (!value.trim() ? "Label is required" : null),
    },
  });

  useEffect(() => {
    if (opened) {
      const values = withDefaults(initialValues);
      form.setValues(values);
      form.resetDirty(values);
    }
  }, [opened]);

  return (
    <Modal opened={opened} onClose={onClose} title={title} centered zIndex={300}>
      <form
        onSubmit={form.onSubmit((values) => {
          onSubmit(values);
          onClose();
          form.reset();
        })}
      >
        <Stack>
          <TextInput label="URL" placeholder="https://example.com" {...form.getInputProps("url")} />
          <TextInput
            label="Boundary RegExp"
            placeholder="^https://(www\.)?example\.com"
            {...form.getInputProps("boundaryRegexp")}
          />
          <TextInput label="Label" placeholder="Marketing site" {...form.getInputProps("label")} />
          <Select
            label="Periodicity"
            data={PERIODICITY_OPTIONS}
            {...form.getInputProps("periodicity")}
          />
          <TagsInput label="Tags" placeholder="Add tags" {...form.getInputProps("tags")} />
          <Switch
            label="Active"
            checked={form.values.active}
            onChange={(event) => form.setFieldValue("active", event.currentTarget.checked)}
          />
          <Group justify="flex-end">
            <Button variant="default" onClick={onClose}>
              Cancel
            </Button>
            <Button type="submit">{submitLabel}</Button>
          </Group>
        </Stack>
      </form>
    </Modal>
  );
};

export default WebsiteFormModal;
