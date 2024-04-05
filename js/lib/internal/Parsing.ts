import { ParsingState } from "../types/LocalId";
import { isNullOrWhiteSpace } from "../util/util";
import { VisVersion, VisVersions } from "../VisVersion";
import { LocalIdParsingErrorBuilder } from "./LocalIdParsingErrorBuilder";

export function parseVisVersion(
    str?: string,
    errorBuilder?: LocalIdParsingErrorBuilder
): VisVersion | undefined {
    if (isNullOrWhiteSpace(str)) {
        errorBuilder?.push(ParsingState.EmptyState);
        return;
    }

    const versionSegmentStart = str.indexOf("vis-");
    if (versionSegmentStart === -1) {
        errorBuilder?.push({
            message: "Failed to find version signature start 'vis-'",
            type: ParsingState.VisVersion,
        });
        return;
    }
    const versionSegmentEnd = str.slice(versionSegmentStart).indexOf("/");
    if (versionSegmentStart === -1) {
        errorBuilder?.push({
            message: "Failed to find version signature end '/'",
            type: ParsingState.VisVersion,
        });
        return;
    }

    const segment = str.slice(
        versionSegmentStart,
        versionSegmentStart + versionSegmentEnd
    );

    const version = VisVersions.tryParse(segment.replace("vis-", ""));

    if (!version) {
        errorBuilder?.push({
            message:
                "Failed to parse VisVersion from segment found: " + segment,
            type: ParsingState.VisVersion,
        });
        return;
    }

    return version;
}
