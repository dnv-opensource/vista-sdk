import { GmodPath } from "..";
import { TraversalHandlerResult } from "./Gmod";

export type TreeHandler<TNode extends TreeNode<TNode>> = (
    node: TNode
) => TraversalHandlerResult;

export type TreeHandlerWithState<TState, TNode> = (
    node: TNode,
    state: TState
) => TraversalHandlerResult;

export type TreeNode<T = unknown> = Omit<
    T,
    "children" | "parent" | "mergedNode"
> &
    Record<"children", TreeNode<T>[]> & {
        parent?: TreeNode<T>;
        mergedNode?: TreeNode<T>;
        key: string;
        path: GmodPath;
    };

export type StrippedNode<T> = Omit<
    TreeNode<T>,
    "children" | "parent" | "mergedNode"
>;

export type InitNode<T> = Record<"children", TreeNode<T>[]> & {
    parent?: TreeNode<T>;
    mergedNode?: TreeNode<T>;
    key: string;
    path: GmodPath;
};

export type FormatNode<TNode extends TreeNode<TNode>> = (
    baseNode: InitNode<TNode>
) => TNode;
