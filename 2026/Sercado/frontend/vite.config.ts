import { defineConfig, loadEnv } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig(({ mode }) => {
  const env = loadEnv(mode, process.cwd(), "");

  const isDocker = process.env.CHOKIDAR_USEPOLLING === "true";
  const backendTarget = isDocker
    ? "http://backend:8080"
    : `http://localhost:${env.API_HOST_PORT || 9000}`;

  console.log(`Backend API Url is set to: ${backendTarget}`);

  return {
    plugins: [react()],
    server: {
      host: "0.0.0.0",
      proxy: {
        "/api": {
          target: backendTarget,
          changeOrigin: true,
        },
        "/graphql": {
          target: backendTarget,
          changeOrigin: true,
        },
      },
    },
  };
});
