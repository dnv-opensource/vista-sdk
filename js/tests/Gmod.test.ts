import { Gmod, GmodNode, GmodPath, VisVersion, VisVersions } from "../lib";
import { TraversalHandlerResult } from "../lib/types/Gmod";
import { naturalSort } from "../lib/util/util";
import { VIS } from "../lib/VIS";
import { getVIS, getVISMap } from "./Fixture";

const version = VIS.latestVisVersion;
const testVersions = VisVersions.all;

beforeAll(async () => {
    await getVISMap();
});

it.each(testVersions)("Gmod loads %s", (version) => {
    const { gmod } = getVIS(version);

    expect(gmod).toBeTruthy();
    expect(gmod.tryGetNode("400a")).toBeTruthy();
});

it("Gmod sorted", () => {
    const { gmod } = getVIS(version);
    const context = { isSorted: true };
    gmod.traverse(
        (_, node, context) => {
            if (node.children.length === 0)
                return TraversalHandlerResult.Continue;

            for (let i = 1; i < node.children.length; i++) {
                const a = node.children[i - 1];
                const b = node.children[i];
                const order = naturalSort(a.toString(), b.toString());
                if (context.isSorted && order > 0) {
                    context.isSorted = false;
                }
            }
            return TraversalHandlerResult.Continue;
        },
        { state: context }
    );
    expect(context.isSorted).toBe(true);
});

it("Gmod node equality", () => {
    const { gmod, locations } = getVIS(version);

    const node1 = gmod.getNode("411.1");
    const node2 = gmod.getNode("411.1");

    expect(node1).toEqual(node2);
    expect(node1).toBe(node2);

    const node3 = node2.withLocation(locations.parse("1"));
    expect(node1).not.toEqual(node3);
    expect(node1).not.toBe(node3);

    const node4 = node2.clone();
    expect(node1).toEqual(node4);
    expect(node1).not.toBe(node4);
});

const expectedMaxes = new Map<VisVersion, { max: string; count: number }>([
    [VisVersion.v3_4a, { max: "C1053.3114", count: 6420 }],
    [VisVersion.v3_5a, { max: "C1053.3114", count: 6557 }],
    [VisVersion.v3_6a, { max: "C1053.3114", count: 6557 }],
    [VisVersion.v3_7a, { max: "H346.11113", count: 6672 }],
    [VisVersion.v3_8a, { max: "H346.11113", count: 6335 }],
    [VisVersion.v3_9a, { max: "H346.11113", count: 6553 }],
]);

it.each(testVersions)("Test gmod properties %s", (visVersion) => {
    const { gmod } = getVIS(visVersion);

    expect(gmod).not.toBeNull();

    let nodeCount = 0;
    let currentMinLength = Number.MAX_SAFE_INTEGER;
    let currentMaxLength = 0;
    let minLengthLexiographicallyOrderedNode = null as GmodNode | null;
    let maxLengthLexiographicallyOrderedNode = null as GmodNode | null;
    for (const node of gmod) {
        nodeCount++;
        const code = node.code;
        const len = code.length;

        if (
            minLengthLexiographicallyOrderedNode === null ||
            len < currentMinLength
        ) {
            currentMinLength = len;
            minLengthLexiographicallyOrderedNode = node;
        } else if (len === currentMinLength) {
            if (
                minLengthLexiographicallyOrderedNode !== null &&
                code.localeCompare(minLengthLexiographicallyOrderedNode.code) <
                    0
            ) {
                minLengthLexiographicallyOrderedNode = node;
            }
        }

        if (
            maxLengthLexiographicallyOrderedNode === null ||
            len > currentMaxLength
        ) {
            currentMaxLength = len;
            maxLengthLexiographicallyOrderedNode = node;
        } else if (len === currentMaxLength) {
            if (
                maxLengthLexiographicallyOrderedNode !== null &&
                code.localeCompare(maxLengthLexiographicallyOrderedNode.code) >
                    0
            ) {
                maxLengthLexiographicallyOrderedNode = node;
            }
        }
    }

    expect(minLengthLexiographicallyOrderedNode).not.toBeNull();
    expect(maxLengthLexiographicallyOrderedNode).not.toBeNull();

    expect(minLengthLexiographicallyOrderedNode?.code.length).toBe(2);
    expect(minLengthLexiographicallyOrderedNode?.code).toBe("VE");

    expect(maxLengthLexiographicallyOrderedNode?.code.length).toBe(10);

    const expectedValues = expectedMaxes.get(visVersion);
    expect(expectedValues).toBeDefined();

    expect(maxLengthLexiographicallyOrderedNode?.code).toBe(
        expectedValues?.max
    );
    expect(nodeCount).toBe(expectedValues?.count);
});

it.each(testVersions)("Gmod lookup %s", async (visVersion) => {
    const { gmod } = getVIS(visVersion);

    expect(gmod).not.toBeNull();

    const gmodDto = await VIS.instance.getGmodDto(visVersion);
    expect(gmodDto).not.toBeNull();

    {
        const seen = new Set<string>();
        let counter = 0;
        for (const node of gmodDto.items) {
            expect(node?.code).not.toBeNull();
            expect(seen.has(node.code)).toBe(false);
            seen.add(node.code);

            expect(node.code).not.toBeNull();
            const foundNode = gmod.tryGetNode(node.code);
            expect(foundNode).not.toBeNull();
            expect(node.code).toBe(foundNode?.code);
            counter++;
        }
    }

    {
        const seen = new Set<string>();
        let counter = 0;

        for (const node of gmod) {
            expect(node?.code).not.toBeNull();
            expect(seen.has(node.code)).toBe(false);
            seen.add(node.code);

            expect(node.code).not.toBeNull();
            const foundNode = gmod.tryGetNode(node.code);
            expect(foundNode).not.toBeNull();
            expect(node.code).toBe(foundNode?.code);
            counter++;
        }

        expect(counter).toBe(gmodDto.items.length);
    }

    expect(gmod.tryGetNode("ABC")).toBeUndefined();
    expect(gmod.tryGetNode(null!)).toBeUndefined();
    expect(gmod.tryGetNode("SDFASDFSDAFb")).toBeUndefined();
    expect(gmod.tryGetNode("✅")).toBeUndefined();
    expect(gmod.tryGetNode("a✅b")).toBeUndefined();
    expect(gmod.tryGetNode("ac✅bc")).toBeUndefined();
    expect(gmod.tryGetNode("✅bc")).toBeUndefined();
    expect(gmod.tryGetNode("a✅")).toBeUndefined();
    expect(gmod.tryGetNode("ag✅")).toBeUndefined();
});

it.each(testVersions)("Gmod rootnode children", (version) => {
    const { gmod } = getVIS(version);

    const node = gmod.rootNode;

    expect(node.children).not.toHaveLength(0);
});

it("Normal assignments", () => {
    const { gmod } = getVIS(VisVersion.v3_4a);

    const node1 = gmod.getNode("411.3");

    expect(node1.productType).toBeTruthy();
    expect(node1.productSelection).toBeFalsy();

    const node2 = gmod.getNode("H601");
    expect(node2.productType).toBeFalsy();
});

it("Product selection", () => {
    const { gmod } = getVIS(VisVersion.v3_4a);

    const node = gmod.getNode("CS1");

    expect(node.isProductSelection);
});

const testMappabilityData = [
    { input: "VE", output: false },
    { input: "300a", output: false },
    { input: "300", output: true },
    { input: "411", output: true },
    { input: "410", output: true },
    { input: "651.21s", output: false },
    { input: "924.2", output: true },
    { input: "411.1", output: false },
    { input: "C101", output: true },
    { input: "CS1", output: false },
    { input: "C101.663", output: true },
    { input: "C101.4", output: true },
    { input: "C101.21s", output: false },
    { input: "F201.11", output: true },
    { input: "C101.211", output: false },
];

it("Mappability", () => {
    const { gmod } = getVIS(VisVersion.v3_4a);

    testMappabilityData.forEach(({ input, output }) => {
        const node = gmod.getNode(input);
        expect(node.isMappable).toBe(output);
    });
});

it("Full traversal", () => {
    const { gmod } = getVIS(version);

    const context: {
        paths: Map<string, GmodPath>;
        readonly maxTraversalOccurrence: number;
        maxOccurrence: number;
    } = {
        paths: new Map(),
        maxTraversalOccurrence: 1,
        maxOccurrence: 0,
    };

    const punctureTests = ["I121", "651.21s", "F201.11"];

    gmod.traverse(
        (parents, node, context) => {
            if (
                punctureTests.includes(node.code) &&
                !context.paths.has(node.code)
            ) {
                context.paths.set(node.code, new GmodPath(parents, node));
            }

            var skipOccurenceCheck = Gmod.isProductSelectionAssignment(
                parents.length > 0 ? parents[parents.length - 1] : undefined,
                node
            );
            if (skipOccurenceCheck) return TraversalHandlerResult.Continue;
            var occ = occurrences(parents, node);
            if (occ > context.maxOccurrence) context.maxOccurrence = occ;
            return TraversalHandlerResult.Continue;
        },
        { state: context }
    );
    expect(context.maxOccurrence).toEqual(context.maxTraversalOccurrence);
    expect(context.paths.size).toBe(punctureTests.length);
});

it("Full traversal with options", () => {
    const { gmod } = getVIS(version);

    const context: {
        readonly maxTraversalOccurrence: number;
        maxOccurrence: number;
    } = {
        maxTraversalOccurrence: 2,
        maxOccurrence: 0,
    };

    gmod.traverse(
        (parents, node, context) => {
            var skipOccurenceCheck = Gmod.isProductSelectionAssignment(
                parents.length > 0 ? parents[parents.length - 1] : undefined,
                node
            );
            if (skipOccurenceCheck) return TraversalHandlerResult.Continue;
            var occ = occurrences(parents, node);
            if (occ > context.maxOccurrence) context.maxOccurrence = occ;
            return TraversalHandlerResult.Continue;
        },
        {
            maxTraversalOccurrence: context.maxTraversalOccurrence,
            state: context,
            rootNode: gmod.getNode("411.1"),
        }
    );
    expect(context.maxOccurrence).toEqual(context.maxTraversalOccurrence);
});

it("Partial traversal", () => {
    const { gmod } = getVIS(version);

    const context = { count: 0 };
    const stop = 5;

    gmod.traverse(
        (_, __, context) => {
            if (context.count === stop) {
                return TraversalHandlerResult.Stop;
            }
            context.count++;
            return TraversalHandlerResult.Continue;
        },
        { state: context }
    );

    expect(context.count).toBe(stop);
});

it("Full traversal from", () => {
    const { gmod } = getVIS(version);

    const context = { invalidPaths: [] as GmodPath[] };
    const rootCode = "400a";

    gmod.traverse(
        (parents, node, context) => {
            if (!(parents.length === 0 || parents[0].code === rootCode)) {
                context.invalidPaths.push(new GmodPath(parents, node));
                return TraversalHandlerResult.Stop;
            }
            return TraversalHandlerResult.Continue;
        },
        {
            rootNode: gmod.getNode(rootCode),
            state: context,
        }
    );

    expect(context.invalidPaths.length).toBe(0);
});

function occurrences(parents: GmodNode[], node: GmodNode) {
    return parents.reduce((acc, parent) => {
        if (parent.code == node.code) return acc + 1;
        return acc;
    }, 0);
}
