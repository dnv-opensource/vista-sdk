export interface PmodDto {
    schemaVersion: string;
    root: string;
    info: PmodInfoDto;
    items: PmodNodeDto[];
    relations: [string, string][];
}

export interface PmodInfoDto {
    vessel_name: string;
    vessel_id: string;
    imo_number: number;
    timestamp: string;
}

export interface PmodNodeDto {
    category: string;
    id: string;
    code: string;
    name: string;
    location: string;
    duty: string;
    pmodSource: string;
}
