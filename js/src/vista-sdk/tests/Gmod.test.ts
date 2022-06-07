import { VisVersion } from "../lib";
import { VIS } from "../lib/VIS";

describe("Gmod", () => {
    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const gmodPromise = vis.getGmod(version);

    test("Gmod loads", async () => {
        const gmod = await gmodPromise;

        expect(gmod).toBeTruthy();
        expect(gmod.tryGetNode("400a")).toBeTruthy();
    });

    test("Gmod node equality", async () => {
        const gmod = await gmodPromise;

        const node1 = gmod.getNode("400a");
        const node2 = gmod.getNode("400a");

        expect(node1).toEqual(node2);
        expect(node1).toBe(node2);

        const node3 = node2.withLocation("1");
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

    // test("Normal assignments", async () => {
    //     const gmod = await gmodPromise;

    //     const node1 = gmod.getNode("411.3");

    //     expect(node1.productType).toBeTruthy();
    //     expect(node1.productSelection).toBeFalsy();

    //     const node2 = gmod.getNode("H601");
    //     expect(node2.productType).toBeFalsy();
    // });

    // test("Product selection", async () => {
    //     const gmod = await gmodPromise;

    //     const node = gmod.getNode("CS1");

    //     expect(node.isProductSelection);
    // });

    // const testMappabilityData = [
    //     { input: "VE", output: true},
    //     { input: "300a", output: false},
    //     { input: "300", output: true},
    //     { input: "411", output: true},
    //     { input: "410", output: true},
    //     { input: "651.21s", output: false},
    //     { input: "942.2", output: true},
    //     { input: "411.1", output: false},
    //     { input: "C101", output: true},
    //     { input: "CS1", output: false},
    //     { input: "C101.663", output: true},
    //     { input: "C101.4", output: true},
    //     { input: "C101.21s", output: false},
    //     { input: "F201.11", output: true},
    //     { input: "C101.211", output: false},
    // ];

    // test("Mappability", async () => {
    //     const gmod = await gmodPromise;

    //     testMappabilityData.forEach(({input, output}) => {
    //         const node = gmod.getNode(input);
    //         console.log(node.code);
    //         expect(node.isMappable).toBe(output);
    //     });
    // });
});
