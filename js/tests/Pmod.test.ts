import * as testData from "../../testdata/PmodData.json";
import {
    ImoNumber,
    LocalId,
    NotRelevant,
    Pmod,
    PmodNode,
    VisVersion,
    VisVersions,
} from "../lib";
import { TraversalHandlerResult } from "../lib/types/Gmod";
import { Ok } from "../lib/types/Result";
import { StrippedNode } from "../lib/types/Tree";
import { getVIS, getVISMap } from "./Fixture";

// Used for testing
type VmodNode = {
    key: string;
    children: VmodNode[];
    depth: number;
    skip: boolean;
    merge: boolean;
    code: string;
};

describe("Pmod", () => {
    const version = VisVersion.v3_4a;

    beforeAll(() => {
        return getVISMap();
    });

    var testDataArr = Object.entries(testData).filter(
        ([k, _]) => k !== "default"
    );
    var fullPaths = testDataArr.flatMap<[VisVersion, string]>(
        ([version, data]) => {
            var visVersion = VisVersions.parse(version);
            return data.fullPaths.map<[VisVersion, string]>((fullPath) => [
                visVersion,
                fullPath,
            ]);
        }
    );

    it.each(testDataArr)("From LocalIds", (visVersion, testData) => {
        var version = VisVersions.parse(visVersion);
        const { gmod, codebooks: codeBooks, locations } = getVIS(version);

        const localIds = testData.localIds.map((localIdStr) =>
            LocalId.parse(localIdStr, gmod, codeBooks, locations)
        );

        const maxDepth = Math.max(
            ...localIds
                .flatMap((l) => [l.primaryItem, l.secondaryItem])
                .map((p) => (p ? p.getFullPath().length - 1 : 0))
        );

        const pmod = Pmod.createFromLocalIds(VisVersion.v3_4a, localIds, {
            imoNumber: ImoNumber.create(1234567),
        });

        const cs1Instances = pmod.getNodesByCode("CS1");

        for (let i = 0; i < cs1Instances.length - 1; i++) {
            const currInstance = cs1Instances[i];
            const nextInstance = cs1Instances[i + 1];
            expect(currInstance).not.toBe(nextInstance);
        }

        expect(pmod.info?.imoNumber?.value).toEqual(1234567);
        expect(pmod.rootNode.toString()).toEqual("VE");
        expect(pmod.rootNode.node.children.length).toBeGreaterThan(0);
        expect(pmod.maxDepth).toEqual(maxDepth);
        expect(pmod.isValid).toBeTruthy();
    });

    it.each(testDataArr)("Traverse pmod from node", (visVersion, testData) => {
        var version = VisVersions.parse(visVersion);
        const { gmod, codebooks: codeBooks, locations } = getVIS(version);

        const localIds = testData.localIds.map((localIdStr) =>
            LocalId.parse(localIdStr, gmod, codeBooks, locations)
        );

        const pmod = Pmod.createFromLocalIds(VisVersion.v3_4a, localIds, {
            imoNumber: ImoNumber.create(1234567),
        });

        const rootNodeCode = "411.1";

        const rootNode = pmod.getNodesByCode(rootNodeCode);
        const rootPath = gmod.tryParseFromFullPath(rootNode[0].id, locations);

        const context = {
            nodes: [] as VmodNode[],
            nodeMap: new Map<string, VmodNode>(
                rootNode[0].parents
                    .reverse()
                    .reduce<[string, VmodNode][]>((c, p, index, arr) => {
                        if (index === 0) {
                            c.push([
                                p.id,
                                {
                                    key: p.id,
                                    children: [],
                                    code: p.code,
                                    depth: 0,
                                    merge: false,
                                    skip: false,
                                },
                            ]);
                            return c;
                        }

                        c.unshift([
                            p.id,
                            {
                                key: p.id,
                                children: [c[index - 1][1]],
                                code: p.code,
                                depth: index,
                                merge: false,
                                skip: false,
                            },
                        ]);
                        return c;
                    }, [])
                    .reverse()
            ),
            iter: 0,
        };

        const createKey = (parents: PmodNode[], node: PmodNode) =>
            parents
                .concat(node)
                .map((n) => n.toString())
                .join("/");

        pmod.traverse(
            (parents, node, context) => {
                const key = createKey(parents, node);

                if (context.nodeMap.has(key))
                    return TraversalHandlerResult.Continue;

                const vmodNode: VmodNode = {
                    key,
                    children: [],
                    depth: node.depth,
                    code: node.code,
                    merge: false,
                    skip: false,
                };

                context.nodeMap.set(key, vmodNode);
                if (context.nodes.length === 0) {
                    context.nodes.push(vmodNode);
                    return TraversalHandlerResult.Continue;
                }

                let parentKey = createKey(
                    parents,
                    parents.splice(parents.length - 1, 1)[0]
                );

                let vmodParent = context.nodeMap.get(parentKey);
                if (!vmodParent)
                    throw new Error("Unexpected state: should find parent");

                vmodParent.children.push(vmodNode);
                return TraversalHandlerResult.Continue;
            },
            { state: context, fromPath: rootPath }
        );

        expect(context.nodes[0].code).toEqual(rootNodeCode);
    });

    it.each(fullPaths)("Tree parse paths %s", (visVersion, testPath) => {
        var version = VisVersions.parse(visVersion);
        const { gmod, locations } = getVIS(version);

        const path = gmod.parseFromFullPath(testPath, locations);
        expect(path).toBeTruthy();
    });

    it.each(testDataArr)("Tree", (visVersion, testData) => {
        var version = VisVersions.parse(visVersion);
        const { gmod, locations } = getVIS(version);

        const paths = testData.fullPaths.map((path) =>
            gmod.parseFromFullPath(path, locations)
        );

        const pmod = Pmod.createFromPaths(VisVersion.v3_6a, paths, {
            imoNumber: ImoNumber.create(1234567),
        });

        const rootPath = gmod.parseFromFullPath(
            "VE/500a/510/511/511.1/511.1i/511.11/CS1/C101",
            locations
        );

        type VmodNode = StrippedNode<{ another: string }> & {
            children: VmodNode[];
            parent?: VmodNode;
            mergedNode?: VmodNode;
        };

        let result = pmod.getVisualizableTreeNodes<VmodNode>(
            (node, _) => {
                expect(node.another).toEqual(node.key);
                return TraversalHandlerResult.Continue;
            },
            {
                fromPath: rootPath,
                withoutLocation: true,
                formatNode: (node) => ({ ...node, another: node.key }),
            }
        );

        expect(result instanceof Ok).toBeTruthy();
        if (result instanceof Ok) {
            expect(result.value.path.toString()).toEqual("511.1");
        }

        result = pmod.getVisualizableTreeNodes(
            (node, _) => {
                return TraversalHandlerResult.Continue;
            },
            { fromPath: rootPath, withoutLocation: false }
        );

        expect(result instanceof NotRelevant).toBeTruthy();
        if (result instanceof NotRelevant) {
            expect(result.value.path.toString()).toEqual("VE");
        }

        // const resolveMergedNodes = (n: TreeNode) => {
        //     const items = [n.path.node.toString()];

        //     if (n.mergedNode) items.unshift(n.mergedNode.path.node.toString());

        //     return items.join(" | ");
        // };

        // const print = (n: TreeNode, depth = 0): string => {
        //     const indents = Array.from(Array(depth).keys())
        //         .map(() => "\t")
        //         .join();

        //     return (
        //         indents +
        //         resolveMergedNodes(n) +
        //         "\n" +
        //         n.children.map((n) => print(n, depth + 1)).join("\n")
        //     );
        // };

        // console.log(nodes.map(print).join("\n"));
    });
});
