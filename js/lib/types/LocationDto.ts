export interface LocationsDto {
    visRelease: string;
    items: RelativeLocationsDto[];
}

export interface RelativeLocationsDto {
    code: string;
    name: string;
    definition?: string | null;
}
