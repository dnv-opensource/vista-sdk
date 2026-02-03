import * as fs from "fs";
import * as path from "path";

const CACHE_DIR = path.join(__dirname, ".vis-cache");

export default async function globalTeardown() {
    // Clean up cache directory
    if (fs.existsSync(CACHE_DIR)) {
        fs.rmSync(CACHE_DIR, { recursive: true });
    }
}
