import { existsSync, readFileSync } from "fs";
import { join } from "path";
import { Client } from "../../lib/Client";
import { VisVersion, VisVersions } from "../../lib/VisVersion";
import { CodebooksDto } from "../../lib/types/CodebookDto";
import { GmodDto } from "../../lib/types/GmodDto";
import { LocationsDto } from "../../lib/types/LocationDto";

const cacheDir = join(__dirname, ".vis-cache");

// Cache storage for DTOs loaded from disk
const dtoCache = new Map<
    VisVersion,
    { gmodDto: GmodDto; codebooksDto: CodebooksDto; locationsDto: LocationsDto }
>();

// Load all cached DTOs from disk
if (existsSync(cacheDir)) {
    for (const version of VisVersions.all) {
        const cachePath = join(cacheDir, `${version}.json`);
        if (existsSync(cachePath)) {
            try {
                const data = readFileSync(cachePath, "utf-8");
                const dtos = JSON.parse(data);
                dtoCache.set(version, dtos);
            } catch {
                // Ignore errors, Client will fetch from network if needed
            }
        }
    }
}

// Override Client methods to use disk cache
const originalVisGetGmod = Client.visGetGmod.bind(Client);
Client.visGetGmod = async (version: VisVersion): Promise<GmodDto> => {
    const cached = dtoCache.get(version);
    if (cached) return cached.gmodDto;
    console.warn(
        `[setupTests] Cache miss for Gmod ${version}, fetching from network`
    );
    return originalVisGetGmod(version);
};

const originalVisGetCodebooks = Client.visGetCodebooks.bind(Client);
Client.visGetCodebooks = async (version: VisVersion): Promise<CodebooksDto> => {
    const cached = dtoCache.get(version);
    if (cached) return cached.codebooksDto;
    console.warn(
        `[setupTests] Cache miss for Codebooks ${version}, fetching from network`
    );
    return originalVisGetCodebooks(version);
};

const originalVisGetLocation = Client.visGetLocation.bind(Client);
Client.visGetLocation = async (version: VisVersion): Promise<LocationsDto> => {
    const cached = dtoCache.get(version);
    if (cached) return cached.locationsDto;
    console.warn(
        `[setupTests] Cache miss for Locations ${version}, fetching from network`
    );
    return originalVisGetLocation(version);
};
