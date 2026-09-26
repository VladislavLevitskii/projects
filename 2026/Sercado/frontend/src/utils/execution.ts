import type { Execution } from "../types";

export const getExecutionTime = (execution: Execution | undefined) =>
  execution?.startTime ? new Date(execution.startTime).getTime() : Infinity;

export const findLatestExecution = (executions: Execution[]): Execution | undefined =>
  executions.reduce<Execution | undefined>(
    (latest, execution) =>
      !latest || getExecutionTime(execution) > getExecutionTime(latest) ? execution : latest,
    undefined,
  );
