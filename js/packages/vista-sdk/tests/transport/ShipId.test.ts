import { ShipId } from "../../lib/transport/domain";

describe("ShipId", () => {
    const validImoStr = "IMO1234567";
    const otherString = "myVessel";
    const invalidImoStr = "IMO1234568";
    const invalidImoStartStr = "IM1234567";

    it("ParseValid IMO", () => {
        const shipId = ShipId.parse(validImoStr);

        expect(shipId.imoNumber).toBeTruthy();
        expect(shipId.imoNumber!.value).toEqual(1234567);
    });

    it("ParseInvalidValid IMO start", () => {
        const shipId = ShipId.parse(invalidImoStartStr);

        expect(shipId.imoNumber).toBeUndefined();
        expect(shipId.otherId).toBeTruthy();
        expect(shipId.otherId).toEqual(invalidImoStartStr);
    });

    it("ParseInvalidValid IMO number", () => {
        const shipId = ShipId.parse(invalidImoStr);

        expect(shipId.imoNumber).toBeUndefined();
        expect(shipId.otherId).toBeTruthy();
        expect(shipId.otherId).toEqual(invalidImoStr);
    });

    it("ParseValid Other", () => {
        const shipId = ShipId.parse(otherString);

        expect(shipId.otherId).toBeTruthy();
        expect(shipId.otherId).toEqual(otherString);
    });
});
