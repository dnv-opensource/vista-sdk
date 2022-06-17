import * as fs from "fs-extra";
import { EmbeddedResource } from "./lib/source-generator/EmbeddedResource";
import { VisGenerator } from "./lib/source-generator/VisGenerator";

module.exports = (async () => {
    /* Copy resources */
    const resourceDir = "../../../resources";
    if (fs.existsSync(resourceDir)) {
        console.log("> Copy resource dir");
        fs.copySync(resourceDir, EmbeddedResource.RESOURCE_DIR, {
            overwrite: true,
        });
    }

    /* Generate VisVersion.ts */
    const out_path = "./VisVersion.ts";
    // Remove existing file
    if (fs.existsSync(out_path)) fs.unlinkSync(out_path);

    const visVersions = await EmbeddedResource.getGmodVisVersions();
    if (!visVersions) return;
    const file = VisGenerator.assembleVisVersionFile(visVersions);
    console.log("> Write VisVersion.ts");

    fs.writeFileSync(out_path, file);
})();
