import { ImoNumber, LocalId, Pmod, VIS, VisVersion } from "../lib";
import * as testData from "../../../../testdata/PmodData.json";

describe("Pmod", () => {
    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const gmodPromise = vis.getGmod(version);
    const codebooksPromise = vis.getCodebooks(version);

    test("From LocalIds", async () => {
        const gmod = await gmodPromise;
        const codeBooks = await codebooksPromise;

        const localIds = testData.localIds.map((localIdStr) =>
            LocalId.parse(localIdStr, gmod, codeBooks)
        );

        const maxDepth = Math.max(
            ...localIds
                .flatMap((l) => [l.primaryItem, l.secondaryItem])
                .map((p) => (p ? p.getFullPath().length - 1 : 0))
        );

        const pmod = Pmod.createFromLocalIds(VisVersion.v3_4a, localIds, {
            imoNumber: new ImoNumber(1234567),
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
});
