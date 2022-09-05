import {
    Gmod,
    Codebooks,
    LocalIdParsingErrorBuilder,
    UniversalIdBuilder,
    ParsingState,
    ImoNumber,
    LocalIdBuilder,
} from ".";

export class UniversalIdParser {
    public static parse(
        universalId: string,
        gmod: Gmod,
        codebooks: Codebooks,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        const result = this.tryParse(
            universalId,
            gmod,
            codebooks,
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
                    if (!segment.startsWith("IMO")) {
                        errorBuilder?.push({
                            type: ParsingState.IMONumber,
                            message: "Couldnt find IMO in segment",
                        });
                        break;
                    }

                    const numberMatch = /\d+/.exec(segment);
                    if (!numberMatch || numberMatch.length !== 1) {
                        errorBuilder?.push({
                            type: ParsingState.IMONumber,
                            message: "Couldnt find valid numbers in segment",
                        });
                        break;
                    }

                    const numbersToCheck = +numberMatch[0];

                    if (isNaN(numbersToCheck)) {
                        errorBuilder?.push({
                            type: ParsingState.IMONumber,
                            message: "Failed to parse numbers from segment",
                        });
                        break;
                    }
                    try {
                        const imo = new ImoNumber(numbersToCheck);
                        imoNumber = imo;
                    } catch (error) {
                        errorBuilder?.push({
                            type: ParsingState.IMONumber,
                            message: (error as unknown as Error).message,
                        });
                    }
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
}
