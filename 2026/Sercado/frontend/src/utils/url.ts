export const getDomain = (value: string) => {
  try {
    return new URL(value).host;
  } catch {
    return value;
  }
};

export const normalizeUrl = (value: string) => {
  try {
    const url = new URL(value);
    return url.toString().replace(/\/$/, "");
  } catch {
    return value.replace(/\/$/, "");
  }
};

export const escapeRegExp = (value: string) => value.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");

export const buildBoundaryRegexp = (value?: string) => {
  if (!value) {
    return "";
  }
  return `^https?://(www\\.)?${escapeRegExp(getDomain(value))}`;
};
