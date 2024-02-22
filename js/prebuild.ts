import * as fs from "fs-extra";
// import { compile } from "json-schema-to-typescript";
// import DataListDto from "../schemas/json/experimental/DataList.schema.json";
// import TimeSeriesDto from "../schemas/json/experimental/TimeSeriesData.schema.json";
import { EmbeddedResource } from "./lib/source-generator/EmbeddedResource";
import { VisGenerator } from "./lib/source-generator/VisGenerator";

module.exports = (async () => {
    // const prepareSchema = (schema: any) => {
    //     if (typeof schema !== "object") return schema;

    //     if (schema.format === "date" || schema.format === "date-time")
    //         return { ...schema, tsType: "Date" };

    //     for (const key in schema) {
    //         if (!schema.hasOwnProperty(key)) continue;
    //         schema[key] = prepareSchema(schema[key]);
    //     }
    //     return schema;
    // };

    // const schemas: {
    //     [k: string]: { schema: any; output: string };
    // } = {
    //     DataListDto: {
    //         schema: DataListDto,
    //         output: "./lib/experimental/transport/json/data-list/DataList.ts",
    //     },
    //     TimeSeriesDto: {
    //         schema: TimeSeriesDto,
    //         output: "./lib/experimental/transport/json/time-series-data/TimeSeriesData.ts",
    //     },
    // };

    // for (const name in schemas) {
    //     if (!schemas.hasOwnProperty(name)) continue;
    //     const { schema, output } = schemas[name];

    //     await compile(prepareSchema(schema), name).then((ts) => {
    //         console.log(ts);

    //         const splits = ts.split("*/");

    //         let last = splits.pop() + "";
    //         last = `export namespace ${name} {\n${last}\n}`;

    //         const out = splits.join("*/") + "*/\n" + last;

    //         return fs.writeFileSync(output, out);
    //     });
    // }

    /* Copy resources */
    const resourceDir = "../resources";
    if (fs.existsSync(resourceDir)) {
        console.log("> Copy resource dir");
        fs.copySync(resourceDir, EmbeddedResource.RESOURCE_DIR, {
            overwrite: true,
        });
    }

    /* Generate VisVersion.ts */
    const out_path = "./lib/VisVersion.ts";
    // Remove existing file
    if (fs.existsSync(out_path)) fs.unlinkSync(out_path);

    const visVersions = await EmbeddedResource.getGmodVisVersions();
    if (!visVersions) return;
    const file = VisGenerator.assembleVisVersionFile(visVersions);
    console.log("> Write VisVersion.ts");

    fs.writeFileSync(out_path, file);
})();
