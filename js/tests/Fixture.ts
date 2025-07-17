import {
    VisVersion,
    Gmod,
    Codebooks,
    Locations,
    VIS,
    VisVersions,
} from "../lib";

export let visMap: Map<
    VisVersion,
    {
        gmod: Gmod;
        codebooks: Codebooks;
        locations: Locations;
    }
> = new Map();

let visPromise: Promise<typeof visMap> | null = null;

export const getVISMap = async () => {
    if (visPromise === null) {
        visPromise = VIS.instance.getVISMap(VisVersions.all).then((m) => {
            visMap = m;
            return visMap;
        });
    }
    return await visPromise;
};

export const getVIS = (version: VisVersion) => {
    return visMap.get(version)!;
};
