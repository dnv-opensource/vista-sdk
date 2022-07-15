import { Gmod } from "./Gmod";
import { ImoNumber } from "./ImoNumber";
import { GmodDto, GmodNodeDto } from "./types/GmodDto";
import { PmodInfo } from "./types/Pmod";
import { PmodDto } from "./types/PmodDto";

export class Pmod {
    private _model: Gmod;
    private _info?: PmodInfo;

    private constructor(model: Gmod, info?: PmodInfo) {
        this._model = model;
        this._info = info;
    }

    public static createFromDto(pmodDto: PmodDto, gmod: Gmod) {
        const gmodDto: GmodDto = {
            items: pmodDto.items.map((item) => {
                const node = gmod.getNode(item.code);
                return {
                    category: node.metadata.category,
                    code: item.code,
                    name: item.name,
                    type: node.metadata.type,
                    commonDefinition: node.metadata.commonDefinition,
                    commonName: item.name,
                    definition: node.metadata.definition,
                    installSubstructure: node.metadata.installSubstructure,
                    normalAssignmentNames: Object.fromEntries(
                        node.metadata.normalAssignmentNames
                    ),
                    location: item.location,
                    id: item.id,
                } as GmodNodeDto;
            }),
            relations: pmodDto.relations.map((r) => [r[0], r[1]]),
            visRelease: gmod.visVersion,
        };
        return new Pmod(new Gmod(gmod.visVersion, gmodDto), {
            imoNumber: new ImoNumber(pmodDto.info.imo_number),
            timestamp: new Date(pmodDto.info.timestamp),
            vesselId: pmodDto.info.vessel_id,
            vesselName: pmodDto.info.vessel_name,
        });
    }

    public static create(gmod: Gmod, info?: PmodInfo) {
        return new Pmod(gmod, info);
    }

    public get model() {
        return this._model;
    }

    public get info() {
        return this._info;
    }
}
