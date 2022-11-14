import {
    Gmod,
    Codebooks,
    LocalIdParsingErrorBuilder,
    UniversalIdBuilder,
    ParsingState,
    ImoNumber,
    LocalIdBuilder,
    VIS,
} from ".";
import { parseVisVersion } from "./internal/Parsing";
import { Locations } from "./Location";

export class UniversalIdParser {
    public static parse(
        universalId: string,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        const result = this.tryParse(
            universalId,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
        if (!result) {
            throw new Error("Failed to parse Universal id: " + universalId);
        }

        return result;
    }

    public static tryParse(
        universalId: string,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): UniversalIdBuilder | undefined {
        const localIdStartIndex = universalId.indexOf("/dnv-v");

        if (localIdStartIndex === -1) {
            errorBuilder?.push({
                message: "Failed to find localId start segment - NamingRule",
                type: ParsingState.NamingRule,
            });
            return;
        }

        const localIdSegment = universalId.slice(localIdStartIndex);

        const universalIdSegment = universalId
            .slice(0, localIdStartIndex + 1)
            .split("");

        // UniversalId Parser
        let namingEntity: string | undefined = undefined;
        let imoNumber: ImoNumber | undefined = undefined;

        const localIdBuilder: LocalIdBuilder | undefined =
            LocalIdBuilder.tryParse(
                localIdSegment,
                gmod,
                codebooks,
                locations,
                errorBuilder
            );

        let nextSegmentIndex = universalIdSegment.indexOf("/");
        let state = ParsingState.NamingEntity;

        while (state <= ParsingState.IMONumber) {
            const segment = universalIdSegment
                .splice(0, nextSegmentIndex)
                .join("");

            switch (state) {
                case ParsingState.NamingEntity:
                    if (segment !== UniversalIdBuilder.namingEntity) {
                        errorBuilder?.push({
                            type: ParsingState.NamingEntity,
                            message:
                                "Naming entity segment didnt match. Found: " +
                                segment,
                        });
                        break;
                    }
                    namingEntity = segment;
                    break;
                case ParsingState.IMONumber:
                    const parsedImo = ImoNumber.tryParse(segment);
                    if (!parsedImo) {
                        errorBuilder?.push({
                            type: ParsingState.IMONumber,
                            message: "Couldnt parse IMO number segment",
                        });
                        break;
                    }

                    imoNumber = parsedImo;
                    break;
            }
            universalIdSegment.splice(0, 1);
            nextSegmentIndex = universalIdSegment.indexOf("/");
            state += 1;
        }

        const builder = UniversalIdBuilder.create(gmod.visVersion)
            .withImoNumber(imoNumber)
            .withLocalId(localIdBuilder);

        return builder;
    }

    public static async parseAsync(
        universalIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        var universalId = await this.tryParseAsync(
            universalIdString,
            errorBuilder
        );

        if (!universalId)
            throw new Error(
                "Couldn't parse local ID from: " + universalIdString
            );

        return universalId;
    }

    public static async tryParseAsync(
        universalIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        const version = parseVisVersion(universalIdString, errorBuilder);
        if (!version) return;

        const gmod = await VIS.instance.getGmod(version);
        const codebooks = await VIS.instance.getCodebooks(version);
        const locations = await VIS.instance.getLocations(version);

        return this.tryParse(
            universalIdString!,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
    }
}
