import { useState } from "react";
import { AppShell, Group, SegmentedControl, Title } from "@mantine/core";
import { ExecutionsProvider } from "./context/ExecutionsContext";
import { GraphProvider } from "./context/GraphContext";
import { WebsitesProvider } from "./context/WebsitesContext";
import ExecutionsView from "./views/ExecutionsView";
import SitesView from "./views/SitesView";
import VisualizationView from "./views/VisualizationView";
import type { PageView } from "./types";
import logoImage from "/favicon.png?url";

const AppContent = () => {
  const [pageView, setPageView] = useState<PageView>("sites");

  return (
    <AppShell header={{ height: 72 }} padding="md">
      <AppShell.Header>
        <Group h="100%" px="md" justify="space-between">
          <Group>
            <img src={logoImage} width={24} height={24} alt="Graph" />
            <Title order={3}>WebCrawler Serĉado</Title>
          </Group>
          <SegmentedControl
            value={pageView}
            onChange={(value) => setPageView(value as PageView)}
            data={[
              { value: "sites", label: "Sites" },
              { value: "executions", label: "Executions" },
              { value: "visualization", label: "Visualization" },
            ]}
          />
        </Group>
      </AppShell.Header>

      <AppShell.Main>
        {pageView === "sites" && <SitesView />}
        {pageView === "executions" && <ExecutionsView />}
        {pageView === "visualization" && <VisualizationView />}
      </AppShell.Main>
    </AppShell>
  );
};

function App() {
  return (
    <WebsitesProvider>
      <ExecutionsProvider>
        <GraphProvider>
          <AppContent />
        </GraphProvider>
      </ExecutionsProvider>
    </WebsitesProvider>
  );
}

export default App;
