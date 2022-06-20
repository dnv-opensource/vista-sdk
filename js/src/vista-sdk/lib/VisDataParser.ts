

import * as fs from "fs-extra";
import * as zlib from "zlib";


export async function writeJsonGzip(file: string): Promise<void> {
    if (!file.endsWith(".gz")) throw new Error("Invalid resource file");

    const jsonStr = await new Promise<string>((res) => {
        const unzip = zlib.createUnzip();
        const inputStream = fs.createReadStream("../../../resources/" + file);
        const stream = inputStream.pipe(unzip);

        const segments: Set<Buffer> = new Set();

        stream.on("data", (data) => {
            segments.add(data);
        });

        stream.on("end", () => {
            const result = Buffer.concat(Array.from(segments)).toString();
            res(result);
        });
    });
    fs.writeFileSync("./lib/resources/" + file.slice(0, -3), jsonStr);
}


fs.readdir("../../../resources").then(files => {
console.log(files)
    files.forEach(async file => {
        await writeJsonGzip(`${file}`);
    })
    fs.writeFileSync("./lib/resources/files.json", JSON.stringify(files.map((x) => x.slice(0,-3))))
});


