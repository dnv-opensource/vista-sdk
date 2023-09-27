import { ILocalId } from "../../../../ILocalId";
import { Version } from "../../../../transport/domain/data-channel/Version";
import { AssetIdentifier } from "../../domain/AssetIdentifier";
import { DataList } from "../../domain/data-list/DataList";
import { DataId } from "../../domain/time-series-data/DataId";
import { DataListDto } from "./DataList";

abstract class IExtension {
    public static toJsonDto: <Domain, Dto>(domain: Domain) => Dto;
    public static toDomainModel: <Domain, Dto>(dto: Dto) => Domain;
}

export class Extensions implements IExtension {
    public static toJsonDto(
        domain: DataList.DataListPackage
    ): DataListDto.DataListPackage {
        const p = domain.package;
        const h = domain.package.header;

        return {
            Package: {
                Header: {
                    Author: h.author,
                    DataListID: {
                        ID: h.dataListId.id,
                        TimeStamp: h.dataListId.timestamp,
                        Version: h.dataListId.version?.toString(),
                    },
                    AssetId: h.assetId.toString(),
                    DateCreated: h.dateCreated,
                    VersionInformation: h.versionInformation
                        ? {
                              NamingRule: h.versionInformation.namingRule,
                              NamingSchemeVersion:
                                  h.versionInformation.namingSchemeVersion,
                              ReferenceURL: h.versionInformation.referenceUrl,
                          }
                        : undefined,
                    ...h.customHeaders,
                },
                DataList: {
                    Data: p.dataList.data.map(DataExtension.toJsonDto),
                },
            },
        };
    }

    public static async toDomainModel(
        dto: DataListDto.DataListPackage
    ): Promise<DataList.DataListPackage> {
        const p = dto.Package;
        const h = dto.Package.Header;

        const datas: DataList.Data[] = [];

        for (let c of p.DataList.Data) {
            const dc = await DataExtension.toDomainModel(c);
            datas.push(dc);
        }

        const {
            AssetId,
            DataListID,
            Author,
            DateCreated,
            VersionInformation,
            ...customHeaders
        } = h;

        return {
            package: {
                header: {
                    dataListId: {
                        id: h.DataListID.ID,
                        timestamp: new Date(h.DataListID.TimeStamp),
                        version: h.DataListID.Version
                            ? Version.parse(h.DataListID.Version)
                            : undefined,
                    },
                    dateCreated: h.DateCreated
                        ? new Date(h.DateCreated)
                        : undefined,
                    assetId: AssetIdentifier.parse(h.AssetId),
                    author: h.Author,
                    versionInformation: h.VersionInformation
                        ? {
                              namingRule: h.VersionInformation.NamingRule,
                              namingSchemeVersion:
                                  h.VersionInformation.NamingSchemeVersion,
                              referenceUrl: h.VersionInformation.ReferenceURL,
                          }
                        : undefined,
                    customHeaders,
                },
                dataList: {
                    data: datas,
                },
            },
        };
    }
}

export class DataExtension {
    public static toJsonDto(c: DataList.Data): DataListDto.Data {
        return {
            DataID: {
                LocalID: c.dataId.localId.toString(),
                ShortID: c.dataId.shortId,
                NameObject: c.dataId.nameObject
                    ? {
                          NamingRule: c.dataId.nameObject.namingRule,
                          ...c.dataId.nameObject.customProperties,
                      }
                    : undefined,
            },
            Property: {
                DataType: {
                    Type: c.property.dataType.type,
                    CalculationPeriod: c.property.dataType.calculationPeriod,
                    UpdateCycle: c.property.dataType.updateCycle,
                },
                Format: {
                    Type: c.property.format.type,
                    Restriction: c.property.format.restriction && {
                        Enumeration: c.property.format.restriction.enumeration,
                        FractionDigits:
                            c.property.format.restriction.fractionDigits,
                        Length: c.property.format.restriction.length,
                        MaxExclusive:
                            c.property.format.restriction.maxExclusive,
                        MaxInclusive:
                            c.property.format.restriction.maxInclusive,
                        MaxLength: c.property.format.restriction.maxLength,
                        MinExclusive:
                            c.property.format.restriction.minExclusive,
                        MinInclusive:
                            c.property.format.restriction.minInclusive,
                        MinLength: c.property.format.restriction.minLength,
                        Pattern: c.property.format.restriction.pattern,
                        TotalDigits: c.property.format.restriction.totalDigits,
                        WhiteSpace: c.property.format.restriction.whiteSpace
                            ? c.property.format.restriction.whiteSpace
                            : undefined,
                    },
                },
                Range: c.property.range
                    ? {
                          Low: c.property.range.low,
                          High: c.property.range.high,
                      }
                    : undefined,
                Unit: c.property.unit
                    ? {
                          UnitSymbol: c.property.unit.unitSymbol,
                          QuantityName: c.property.unit.quantityName,
                          AdditionalProperties: c.property.customProperties,
                      }
                    : undefined,
                QualityCoding: c.property.qualityCoding,
                AlertPriority: c.property.alertPriority,
                Name: c.property.name,
                Remarks: c.property.remarks,
                AdditionalProperties: c.property.customProperties,
            },
        };
    }

    public static async toDomainModel(
        c: DataListDto.Data
    ): Promise<DataList.Data> {
        const dataId = await DataId.parseAsync(c.DataID.LocalID);

        let nameObject: DataList.NameObject | undefined = undefined;
        let unit: DataList.Unit | undefined = undefined;
        if (c.DataID.NameObject) {
            const { NamingRule, ...customProperties } = c.DataID.NameObject;
            nameObject = {
                namingRule: NamingRule,
                customProperties,
            };
        }

        if (c.Property.Unit) {
            const { UnitSymbol, QuantityName, ...unitCustomProperties } =
                c.Property.Unit;
            unit = {
                unitSymbol: UnitSymbol,
                quantityName: QuantityName,
                customProperties: unitCustomProperties,
            };
        }

        const {
            DataType,
            Format,
            Range,
            Unit,
            QualityCoding,
            AlertPriority,
            Name,
            Remarks,
            ...customProperties
        } = c.Property;
        let property: DataList.Property = {
            dataType: {
                type: c.Property.DataType.Type,
                calculationPeriod: c.Property.DataType.CalculationPeriod,
                updateCycle: c.Property.DataType.UpdateCycle,
            },
            format: {
                type: c.Property.Format.Type,
                restriction: c.Property.Format.Restriction && {
                    enumeration: c.Property.Format.Restriction.Enumeration,
                    fractionDigits:
                        c.Property.Format.Restriction.FractionDigits,
                    length: c.Property.Format.Restriction.Length,
                    maxExclusive: c.Property.Format.Restriction.MaxExclusive,
                    maxInclusive: c.Property.Format.Restriction.MaxInclusive,
                    maxLength: c.Property.Format.Restriction?.MaxLength,
                    minExclusive: c.Property.Format.Restriction.MinExclusive,
                    minInclusive: c.Property.Format.Restriction.MinInclusive,
                    minLength: c.Property.Format.Restriction.MinLength,
                    pattern: c.Property.Format.Restriction.Pattern,
                    totalDigits: c.Property.Format.Restriction.TotalDigits,
                    whiteSpace: c.Property.Format.Restriction.WhiteSpace,
                },
            },
            range: c.Property.Range && {
                low: c.Property.Range.Low,
                high: c.Property.Range.High,
            },
            unit,
            qualityCoding: c.Property.QualityCoding,
            alertPriority: c.Property.AlertPriority,
            name: c.Property.Name,
            remarks: c.Property.Remarks,
            customProperties,
        };

        return {
            dataId: {
                localId: dataId.match<ILocalId>(
                    (l) => l,
                    (p) => p,
                    (s) => {
                        throw new Error(
                            `DataPackage local ID not a valid LocalId: ${s}`
                        );
                    }
                ),
                shortId: c.DataID.ShortID,
                nameObject,
            },
            property,
        };
    }
}
