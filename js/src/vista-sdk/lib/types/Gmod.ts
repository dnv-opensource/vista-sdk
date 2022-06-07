import { GmodNode } from "..";
import { Parents } from "../util/Parents";

export type GmodTuple = {
    category: string;
    type: string;
};

export enum TraversalHandlerResult {
    Stop,
    SkipSubtree,
    Continue,
}

export type TraversalHandler = (
    parents: GmodNode[],
    node: GmodNode
) => TraversalHandlerResult;
export type TraversalHandlerWithState<T> = (
    parents: GmodNode[],
    node: GmodNode,
    state: T
) => TraversalHandlerResult;

export type TraversalContext<T> = {
    parents: Parents;
    handler: TraversalHandlerWithState<T>;
    state: T;
};
