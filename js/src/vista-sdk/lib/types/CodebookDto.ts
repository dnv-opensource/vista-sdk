export type CodebookDto = {
  name: string;
  values: {
    [k: string]: string[];
  };
};

export type CodebooksDto = {
  visVersion: string;
  items: CodebookDto[];
};
