import { GmodPath } from "./GmodPath";
import { VIS } from "./VIS";

export class LocalIdItems {
    public primaryItem?: GmodPath;
    public secondaryItem?: GmodPath;

    public clone(): LocalIdItems {
        const items = new LocalIdItems();
        items.primaryItem = this.primaryItem?.clone();
        items.secondaryItem = this.secondaryItem?.clone();
        return items;
    }

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
                for (const { depth, name } of this.primaryItem.getCommonNames(
                    true,
                )) {
                    builder.push("~");
                    const location = this.primaryItem.getNode(depth).location;
                    LocalIdItems.appendCommonName(
                        builder,
                        name,
                        location?.toString(),
                    );
                    builder.push("/");
                }
            }

            if (!!this.secondaryItem) {
                let prefix = "~for.";
                for (const { depth, name } of this.secondaryItem.getCommonNames(
                    true,
                )) {
                    builder.push(prefix);
                    if (prefix !== "~") prefix = "~";
                    const location = this.secondaryItem.getNode(depth).location;
                    LocalIdItems.appendCommonName(
                        builder,
                        name,
                        location?.toString(),
                    );
                    builder.push("/");
                }
            }
        }
    }
    private static appendCommonName(
        builder: string[],
        commonName: string,
        location?: string,
    ) {
        let prev: string | undefined = undefined;
        for (const ch of commonName) {
            if (ch === "/") continue;
            if (prev === " " && ch === " ") continue;

            let current = ch;
            switch (ch) {
                case " ":
                    current = ".";
                    break;
                default:
                    const match = VIS.isISOString(ch);
                    if (!match) {
                        current = ".";
                        break;
                    }
                    current = ch.toLocaleLowerCase();
                    break;
            }
            if (current === "." && prev === ".") continue;
            builder.push(current);
            prev = current;
        }

        if (location && location.length > 0) {
            builder.push(".");
            builder.push(location);
        }
    }

    public map<T>(func: (item: GmodPath | undefined) => T) {
        return [this.primaryItem, this.secondaryItem].map(func);
    }

    public every(func: (item: GmodPath | undefined) => boolean) {
        return [this.primaryItem, this.secondaryItem].every(func);
    }

    public some(func: (item: GmodPath | undefined) => boolean) {
        return [this.primaryItem, this.secondaryItem].some(func);
    }
}
