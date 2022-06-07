export enum ParsingState {
  NamingRule,
  VisVersion,
  PrimaryItem,
  SecondaryItem,
  ItemDescription,
  MetaQty,
  MetaCnt,
  MetaCalc,
  MetaState,
  MetaCmd,
  MetaType,
  MetaPos,
  MetaDetail,
  // For "other" errors
  EmptyState = 100,
  Formatting = 101,
}
