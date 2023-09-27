import { Gmod } from "..";
import { GmodNode } from "../GmodNode";
import { PmodNode } from "../PmodNode";

export const isNullOrWhiteSpace = (s?: string): s is undefined => {
    return !s || !s.trim();
};

export const tryParseInt = (s?: string): number | undefined => {
    if (!s) return;
    return isNaN(+s) ? undefined : +s;
};

export const parseValue = (v: string): [string, number] => {
    // extract number (defaults to 0 if not present)
    const n = +(v.match(/\d+$/) || [0])[0];
    const str = v.replace("" + n, ""); // extract string part
    return [str, n];
};

export const naturalSort = (a: string, b: string) => {
    return a.localeCompare(b, undefined, {
        numeric: true,
        sensitivity: "base",
        ignorePunctuation: true,
    });
};

export const createKey = <T = GmodNode | PmodNode>(parents: T[], node: T) =>
    parents
        .concat(node)
        // @ts-ignore
        .map((n) => n.toString())
        .join("/");
/**
 * @condition TODO
 * * The node is a product selection assignment
 *      * Parent is a Function (category)
 *      * Node is a Product (category) Selection (type)
 * * Node is a composition (type) -- TODO is this correct? Only Pmod?
 * * Node type is selection (type) -- TODO is this correct?
 */
export const isNodeSkippable = (
    parent: GmodNode | PmodNode,
    node: GmodNode | PmodNode
) => {
    const p = parent instanceof PmodNode ? parent.node : parent;
    const n = node instanceof PmodNode ? node.node : node;

    return Gmod.isProductSelectionAssignment(p, n);
};

/**
 * @condition TODO
 * * The node is a product type assignment
 *      * Parent is a Function (category)
 *      * Node is a Product (category) Type (type)
 */
export const isNodeMergeable = (
    parent: GmodNode | PmodNode,
    node: GmodNode | PmodNode
) => {
    const p = parent instanceof PmodNode ? parent.node : parent;
    const n = node instanceof PmodNode ? node.node : node;

    return Gmod.isProductTypeAssignment(p, n);
};
