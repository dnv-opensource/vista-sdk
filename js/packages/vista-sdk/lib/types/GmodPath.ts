import { GmodPath } from "..";
import { Location } from "../Location";

export interface PathNode {
    code: string;
    location?: Location;
}

export interface ParseContext {
    parts: PathNode[];
    toFind: PathNode;
    locations: Map<string, Location>;
    path?: GmodPath;
}
