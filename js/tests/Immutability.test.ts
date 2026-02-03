import { LocationBuilder, VIS } from "../lib";

describe("Immutability test", () => {
    it("GmodNode", async () => {
        const { gmod, locations } = await VIS.instance.getVIS(
            VIS.latestVisVersion
        );
        const node = gmod.getNode("411.1");
        const other = node.withLocation(locations.parse("1"));
        expect(node).not.toEqual(other);
        expect(node).not.toBe(other);
    });

    it("LocationsBuilder", async () => {
        const { locations } = await VIS.instance.getVIS(VIS.latestVisVersion);
        const builder = LocationBuilder.create(locations);
        const location = builder.build();

        let other = builder.withNumber(1);
        let otherLocation = other.build();

        expect(builder).not.toEqual(other);
        expect(builder).not.toBe(other);
        expect(location).not.toEqual(otherLocation);
        expect(location).not.toBe(otherLocation);
    });

    it("GmodPath", async () => {
        const { gmod, locations } = await VIS.instance.getVIS(
            VIS.latestVisVersion
        );
        const path = gmod.parseFromFullPath(
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.3/C101.3i/C101.31/C101.311-1",
            locations
        );

        const other = path.clone();
        expect(path.equals(other)).toEqual(true);
        expect(path).not.toBe(other);

        expect(path.node).toEqual(other.node);
        expect(path.node).not.toBe(other.node);

        const parentIndex = 5; // 411.1
        expect(path.parents[parentIndex]).toEqual(other.parents[parentIndex]);
        expect(path.parents[parentIndex]).not.toBe(other.parents[parentIndex]);

        // Change node
        path.node = path.node.withLocation(locations.parse("1"));
        other.node = other.node.withLocation(locations.parse("2"));

        expect(path.node).not.toEqual(other.node);
        expect(path).not.toEqual(other);

        // Change parent
        path.parents[parentIndex] = path.parents[parentIndex].withLocation(
            locations.parse("1")
        );
        other.parents[parentIndex] = other.parents[parentIndex].withLocation(
            locations.parse("2")
        );

        expect(path.parents[parentIndex]).not.toEqual(
            other.parents[parentIndex]
        );
        expect(path.parents[parentIndex]).not.toBe(other.parents[parentIndex]);
    });

    it("Individualizable sets", async () => {
        const { gmod, locations } = await VIS.instance.getVIS(
            VIS.latestVisVersion
        );
        const path = gmod.parseFromFullPath(
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.3/C101.3i/C101.31/C101.311-1",
            locations
        );
        const sets = path.individualizableSets;
        const selectedIndex = sets.length - 1;
        const set = sets[selectedIndex];
        set.location = locations.parse("2");
        const other = set.build();

        expect(path).not.toEqual(other);
        expect(path).not.toBe(other);
    });
});
