import { GmodPath } from "./GmodPath";

export class LocalIdItems {
    public primaryItem?: GmodPath;
    public secondaryItem?: GmodPath;

    public append(builder: string[], verboseMode?: boolean) {
        if (!this.primaryItem && !this.secondaryItem) return;

        if (!!this.primaryItem) {
            builder.push(this.primaryItem.toString());
            builder.push("/");
        }

        if (!!this.secondaryItem) {
            builder.push("sec/");
            builder.push(this.secondaryItem.toString());
            builder.push("/");
        }

        if (verboseMode) {
            if (!!this.primaryItem) {
                for (const {
                    depth,
                    name,
                } of this.primaryItem.getCommonNames()) {
                    builder.push("~");
                    const location = this.primaryItem.getNode(depth).location;
                    LocalIdItems.appendCommonName(builder, name, location);
                    builder.push("/");
                }
            }

            if (!!this.secondaryItem) {
                let prefix = "~for.";
                for (const {
                    depth,
                    name,
                } of this.secondaryItem.getCommonNames()) {
                    builder.push(prefix);
                    if (prefix !== "~") prefix = "~";
                    const location = this.secondaryItem.getNode(depth).location;
                    LocalIdItems.appendCommonName(builder, name, location);
                    builder.push("/");
                }
            }
        }
    }
    private static appendCommonName(
        builder: string[],
        commonName: string,
        location?: string
    ) {
        let prev: string | undefined = undefined;
        for (const ch of commonName) {
            if (ch === "/") continue;
            if (prev === " " && ch === " ") continue;

            switch (ch) {
                case " ":
                    builder.push(".");
                    break;
                default:
                    builder.push(ch.toLocaleLowerCase());
                    break;
            }
            prev = ch;
        }

        if (!!location) {
            builder.push(".");
            builder.push(location);
        }
    }
}
