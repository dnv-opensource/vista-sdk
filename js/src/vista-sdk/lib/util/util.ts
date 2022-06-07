export const isNullOrWhiteSpace = (s?: string) => {
  return !s || !s.trim();
};

export const tryParseInt = (s?: string): number | undefined => {
  if (!s) return;
  return isNaN(+s) ? undefined : +s;
};

export const parseValue = (v: string): [string, number] => {
  // extract number (defaults to 0 if not present)
  const n = +(v.match(/\d+$/) || [0])[0];
  const str = v.replace('' + n, ''); // extract string part
  return [str, n];
};

export const getEnumValueByStringKey = <T>(enu: T, key: string) => {
  return Object.entries(enu).find(([k]) => k === key)?.[1];
};

export const naturalSort = (a: string, b: string) => {
  return a.localeCompare(b, undefined, { numeric: true, sensitivity: 'base', ignorePunctuation: true });
};
