export enum ParsingState {
  NamingRule,
  VisVersion,
  PrimaryItem,
  SecondaryItem,
  ItemDescription,
  MetaQuantity,
  MetaContent,
  MetaCalculation,
  MetaState,
  MetaCommand,
  MetaType,
  MetaPosition,
  MetaDetail,
  // For "other" errors
  EmptyState = 100,
  Formatting = 101,
}
