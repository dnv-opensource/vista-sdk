import { ImoNumber } from "..";

export type PmodInfo = Partial<{
    vesselName: string;
    vesselId: string;
    imoNumber: ImoNumber;
    timestamp: Date;
}>;
