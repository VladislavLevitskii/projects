/* eslint-disable react-refresh/only-export-components -- context, hook and provider are kept in one file for readability */
import { createContext, useCallback, useContext, useEffect, useMemo, useState } from "react";
import { z } from "zod";
import { executionApiItemSchema, type Execution } from "../types";
import { findLatestExecution } from "../utils/execution";
import { useWebsites } from "./WebsitesContext";

export type ExecutionsContextValue = {
  executions: Execution[];
  latestExecutions: Map<string, Execution>;
  refreshExecutions: () => Promise<void>;
  startExecution: (websiteId: string) => Promise<void>;
  pauseExecution: (websiteId: string) => Promise<void>;
};

const ExecutionsContext = createContext<ExecutionsContextValue | null>(null);

export const useExecutions = () => {
  const context = useContext(ExecutionsContext);
  if (!context) {
    throw new Error("useExecutions must be used within ExecutionsProvider");
  }
  return context;
};

const executionApiResponseSchema = z.object({
  items: z.array(executionApiItemSchema).default([]),
});

export const ExecutionsProvider = ({ children }: { children: React.ReactNode }) => {
  const { websiteMap } = useWebsites();
  const [allExecutions, setAllExecutions] = useState<Execution[]>([]);

  const refreshExecutions = useCallback(async () => {
    try {
      const res = await fetch("/api/executions?pageSize=10000");
      const data = executionApiResponseSchema.parse(await res.json());
      setAllExecutions(
        data.items.map((item) => ({
          id: item.id,
          websiteId: item.webPageId,
          status: item.status === "completed" ? "success" : item.status,
          startTime: item.startTime,
          endTime: item.endTime ?? undefined,
          pagesCrawled: item.sitesCrawled,
        })),
      );
    } catch (e) {
      console.error("Failed to fetch executions", e);
    }
  }, []);

  useEffect(() => {
    refreshExecutions();
  }, [refreshExecutions]);

  // Hide executions of websites that no longer exist (e.g. right after a delete).
  const executions = useMemo(
    () => allExecutions.filter((execution) => websiteMap.has(execution.websiteId)),
    [allExecutions, websiteMap],
  );

  const latestExecutions = useMemo(() => {
    const byWebsite = new Map<string, Execution[]>();
    executions.forEach((execution) => {
      const list = byWebsite.get(execution.websiteId);
      if (list) {
        list.push(execution);
      } else {
        byWebsite.set(execution.websiteId, [execution]);
      }
    });
    const map = new Map<string, Execution>();
    byWebsite.forEach((siteExecutions, websiteId) => {
      const latest = findLatestExecution(siteExecutions);
      if (latest) {
        map.set(websiteId, latest);
      }
    });
    return map;
  }, [executions]);

  const startExecution = useCallback(
    async (websiteId: string) => {
      try {
        await fetch(`/api/executions/${websiteId}/start`, { method: "POST" });
        refreshExecutions();
      } catch (e) {
        console.error("Failed to start execution", e);
      }
    },
    [refreshExecutions],
  );

  const pauseExecution = useCallback(
    async (websiteId: string) => {
      try {
        await fetch(`/api/executions/${websiteId}/pause`, { method: "POST" });
        refreshExecutions();
      } catch (e) {
        console.error("Failed to pause execution", e);
      }
    },
    [refreshExecutions],
  );

  const value: ExecutionsContextValue = useMemo(
    () => ({ executions, latestExecutions, refreshExecutions, startExecution, pauseExecution }),
    [executions, latestExecutions, refreshExecutions, startExecution, pauseExecution],
  );

  return <ExecutionsContext.Provider value={value}>{children}</ExecutionsContext.Provider>;
};
