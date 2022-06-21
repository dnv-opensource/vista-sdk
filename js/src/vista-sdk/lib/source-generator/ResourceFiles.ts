import resourceFiles from "../resources/files.json";

export class ResourceFiles {
    public static readonly RESOURCE_DIR = "../resources";

    public static async readGmodFile<T>(visVersion: string): Promise<T> {
        var resource = resourceFiles.filter(
            (r) => r.includes("gmod") && r.includes(visVersion)
        )[0];

        if (!resource)
            throw new Error(
                "Couldn't find Gmod resource for VIS version: " + visVersion
            );
        console.log(resource);

        var gmod = await import(ResourceFiles.RESOURCE_DIR + "/" + resource);
        return gmod.default;
    }

    public static async readCodebooksFile<T>(visVersion: string): Promise<T> {
        var resource = resourceFiles.filter(
            (r) => r.includes("codebooks") && r.includes(visVersion)
        )[0];

        if (!resource)
            throw new Error(
                "Couldn't find Gmod resource for VIS version: " + visVersion
            );
        console.log(resource);

        var gmod = await import(ResourceFiles.RESOURCE_DIR + "/" + resource);
        return gmod.default;
    }

    public static async getGmodVisVersions(): Promise<string[]> {
        var resources = resourceFiles.filter(
            (r) => r.includes("gmod") && !r.includes("versioning")
        );

        if (resources.length === 0)
            throw new Error("Couldn't find any Gmod resource");

        const versions: string[] = [];

        for (const resource of resources) {
            var gmod = await import(
                ResourceFiles.RESOURCE_DIR + "/" + resource
            );

            versions.push(gmod.default.visRelease);
        }

        return versions;
    }
}
