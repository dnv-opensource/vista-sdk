import { VisVersion, GmodDto, Gmod, Codebooks, GmodNodeDto } from ".";

import resourceFiles from "./resources/files.json";

import codebooks3_4a from "./resources/codebooks-vis-3-4a.json";
import codebooks3_5a from "./resources/codebooks-vis-3-5a.json";
import gmod3_4a from "./resources/gmod-vis-3-4a.json";
import gmod3_5a from "./resources/gmod-vis-3-5a.json";
import { VisVersionExtension, VisVersions } from "./VisVersion";
import { readJson } from "fs-extra";

export class ResourceFiles{
    public static async readGmodFile<T>(visVersion: string): Promise<T> {
        const resource = resourceFiles.filter(
            (r) =>
                r.includes("gmod") &&
                r.includes(visVersion)
            )[0];

        if (!resource)
            throw new Error("Couldn't find Gmod resource for VIS version: " + visVersion);
        console.log(resource);

        var gmod = await import("./resources/" + resource);
        return gmod.default;
    }

    public static async readCodebooksFile<T>(visVersion: string): Promise<T> {
        const resource = resourceFiles.filter(
            (r) =>
                r.includes("codebooks") &&
                r.includes(visVersion)
            )[0];

        if (!resource)
            throw new Error("Couldn't find Gmod resource for VIS version: " + visVersion);
        console.log(resource);

        var gmod = await import("./resources/" + resource);
        return gmod.default;
    }
}


// export const getGmodFile = (visVersion: string): Promise<T> => {
//     const resource = resourceFiles.filter(
//         (r) =>
//             r.includes("gmod") &&
//             r.includes(visVersion) )[0];
//     if (!resource)
//         throw new Error("Couldn't find Gmod resource for VIS version: " + visVersion);
//     const dto : GmodDto();
//     dto.items = gmod3_4a.items;
//     dto.relations = gmod3_4a.relations;
//     dto.visRelease = gmod3_4a.visRelease;



//     //         switch (version){
//     //     case VisVersion.v3_4a:
//     //         return gmod3_4a;
//     //         break;

//     //     case VisVersion.v3_5a:
//     //         const gmod = gmod3_5a;
//     //         break;
//     //     default:

//     // }
// }

