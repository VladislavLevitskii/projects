// Deterministic id -> color mapping so a website keeps the same color across
// renders/incremental graph updates without needing to track assignment order.
const hashString = (value: string): number => {
  let hash = 0;
  for (let i = 0; i < value.length; i++) {
    hash = (hash << 5) - hash + value.charCodeAt(i);
    hash |= 0;
  }
  return Math.abs(hash);
};

export const getWebsiteColor = (websiteId: string | undefined): string => {
  if (!websiteId) {
    return "hsl(0, 0%, 70%)";
  }

  const hue = hashString(websiteId) % 360;
  return `hsl(${hue}, 65%, 48%)`;
};
