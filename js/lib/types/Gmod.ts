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
    maxTraversalOccurrence: number;
};

export type TraversalOptions<T> = {
    rootNode?: GmodNode;
    state?: T;
    /**
     * @summary The maximum number of times a node can occur in a path. The traversal will stop and the first node to reach the limit, and include it as the end node. Increasing this will drastically reduce performance
     * @default 1
     * @example "411.1/C101.63/S206.22/S110.2/C101" = maxTraversalOccurrence 1. The travesal algorithm finds the second instance of C101 in the path, and stop, but include the node in the result.
     */
    maxTraversalOccurrence?: number;
};
