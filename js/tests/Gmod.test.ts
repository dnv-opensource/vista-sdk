import { GmodPath, VisVersion } from "../lib";
import { TraversalHandlerResult } from "../lib/types/Gmod";
import { VIS } from "../lib/VIS";

describe("Gmod", () => {
    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const gmodPromise = vis.getGmod(version);
    const locationsPromise = vis.getLocations(version);

    test("Gmod loads", async () => {
        const gmod = await gmodPromise;

        expect(gmod).toBeTruthy();
        expect(gmod.tryGetNode("400a")).toBeTruthy();
    });

    test("Gmod node equality", async () => {
        const gmod = await gmodPromise;
        const locations = await locationsPromise;

        const node1 = gmod.getNode("400a");
        const node2 = gmod.getNode("400a");

        expect(node1).toEqual(node2);
        expect(node1).toBe(node2);

        const node3 = node2.withLocation(locations.parse("1"));
        expect(node1).not.toEqual(node3);
        expect(node1).not.toBe(node3);

        const node4 = node2.clone();
        expect(node1).toEqual(node4);
        expect(node1).not.toBe(node4);
    });

    test("Gmod rootnode children", async () => {
        const gmod = await gmodPromise;

        const node = gmod.rootNode;

        expect(node.children).not.toHaveLength(0);
    });

    test("Normal assignments", async () => {
        const gmod = await gmodPromise;

        const node1 = gmod.getNode("411.3");

        expect(node1.productType).toBeTruthy();
        expect(node1.productSelection).toBeFalsy();

        const node2 = gmod.getNode("H601");
        expect(node2.productType).toBeFalsy();
    });

    test("Product selection", async () => {
        const gmod = await gmodPromise;

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

    test("Mappability", async () => {
        const gmod = await gmodPromise;

        testMappabilityData.forEach(({ input, output }) => {
            const node = gmod.getNode(input);
            expect(node.isMappable).toBe(output);
        });
    });

    test("Full traversal", async () => {
        const gmod = await gmodPromise;

        const context: {
            paths: Map<string, GmodPath>;
        } = {
            paths: new Map(),
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
                return TraversalHandlerResult.Continue;
            },
            { state: context }
        );
        expect(context.paths.size).toBe(punctureTests.length);
    });

    test("Partial traversal", async () => {
        const gmod = await gmodPromise;

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

    test("Full traversal from", async () => {
        const gmod = await gmodPromise;

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
});
