/* eslint-disable react-refresh/only-export-components -- context, hook and provider are kept in one file for readability */
import { createContext, useCallback, useContext, useEffect, useMemo, useState } from "react";
import { z } from "zod";
import { websiteApiItemSchema, type WebsiteFormValues, type WebsiteRecord } from "../types";

export type WebsitesContextValue = {
  websites: WebsiteRecord[];
  websiteMap: Map<string, WebsiteRecord>;
  websiteOptions: { value: string; label: string }[];
  tagOptions: string[];
  createWebsite: (values: WebsiteFormValues) => Promise<WebsiteRecord>;
  updateWebsite: (id: string, values: Partial<WebsiteFormValues>) => Promise<void>;
  deleteWebsite: (id: string) => Promise<void>;
};

const WebsitesContext = createContext<WebsitesContextValue | null>(null);

export const useWebsites = () => {
  const context = useContext(WebsitesContext);
  if (!context) {
    throw new Error("useWebsites must be used within WebsitesProvider");
  }
  return context;
};

const websiteApiResponseSchema = z.object({
  items: z.array(websiteApiItemSchema).default([]),
});

export const WebsitesProvider = ({ children }: { children: React.ReactNode }) => {
  const [websites, setWebsites] = useState<WebsiteRecord[]>([]);

  const fetchWebsites = useCallback(async () => {
    try {
      const res = await fetch("/api/websites?pageSize=10000");
      const data = websiteApiResponseSchema.parse(await res.json());
      setWebsites(
        data.items.map((item) => ({
          id: item.identifier,
          url: item.url,
          boundaryRegexp: item.regexp,
          periodicity: item.periodicity,
          label: item.label,
          active: item.active,
          tags: item.tags,
        })),
      );
    } catch (e) {
      console.error("Failed to fetch websites", e);
    }
  }, []);

  useEffect(() => {
    fetchWebsites();
  }, [fetchWebsites]);

  const websiteMap = useMemo(() => new Map(websites.map((site) => [site.id, site])), [websites]);

  const websiteOptions = useMemo(
    () => websites.map((site) => ({ value: site.id, label: site.label })),
    [websites],
  );

  const tagOptions = useMemo(() => {
    const tags = new Set<string>();
    websites.forEach((site) => site.tags.forEach((tag) => tags.add(tag)));
    return Array.from(tags);
  }, [websites]);

  const createWebsite = useCallback(
    async (values: WebsiteFormValues) => {
      const payload = {
        label: values.label,
        url: values.url,
        regexp: values.boundaryRegexp,
        periodicity: values.periodicity,
        tags: values.tags,
        active: values.active,
      };
      const res = await fetch("/api/websites", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });
      const data = await res.json();
      await fetchWebsites();

      return {
        id: data.identifier,
        url: data.url,
        boundaryRegexp: data.regexp,
        periodicity: values.periodicity,
        label: data.label,
        active: data.active,
        tags: data.tags || [],
      };
    },
    [fetchWebsites],
  );

  const updateWebsite = useCallback(
    async (id: string, values: Partial<WebsiteFormValues>) => {
      const current = websiteMap.get(id);
      if (!current) return;
      const payload = {
        label: values.label ?? current.label,
        url: values.url ?? current.url,
        regexp: values.boundaryRegexp ?? current.boundaryRegexp,
        periodicity: values.periodicity ?? current.periodicity,
        tags: values.tags ?? current.tags,
        active: values.active ?? current.active,
      };
      await fetch(`/api/websites/${id}`, {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });
      await fetchWebsites();
    },
    [websiteMap, fetchWebsites],
  );

  // Executions and graph nodes of a deleted website are filtered out downstream
  // against `websiteMap`, so there's no need to refetch them here.
  const deleteWebsite = useCallback(
    async (id: string) => {
      await fetch(`/api/websites/${id}`, { method: "DELETE" });
      await fetchWebsites();
    },
    [fetchWebsites],
  );

  const value: WebsitesContextValue = useMemo(
    () => ({
      websites,
      websiteMap,
      websiteOptions,
      tagOptions,
      createWebsite,
      updateWebsite,
      deleteWebsite,
    }),
    [websites, websiteMap, websiteOptions, tagOptions, createWebsite, updateWebsite, deleteWebsite],
  );

  return <WebsitesContext.Provider value={value}>{children}</WebsitesContext.Provider>;
};
