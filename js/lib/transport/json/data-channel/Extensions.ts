import { LocalId } from "../../..";
import { DataChannelList, ShipId } from "../../domain";
import { Version } from "../../domain/data-channel/Version";
import { DataChannelListDto } from "./DataChannelList";

abstract class IExtension {
    public static toJsonDto: <Domain, Dto>(domain: Domain) => Dto;
    public static toDomainModel: <Domain, Dto>(dto: Dto) => Domain;
}

export class Extensions implements IExtension {
    public static toJsonDto(
        domain: DataChannelList.DataChannelListPackage
    ): DataChannelListDto.DataChannelListPackage {
        const p = domain.package;
        const h = domain.package.header;
        return {
            Package: {
                Header: {
                    Author: h.author,
                    DataChannelListID: {
                        ID: h.dataChannelListId.id,
                        TimeStamp: h.dataChannelListId.timestamp,
                        Version: h.dataChannelListId.version?.toString(),
                    },
                    ShipID: h.shipId.toString(),
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
                DataChannelList: {
                    DataChannel: p.dataChannelList.dataChannel.map(
                        DataChannelExtension.toJsonDto
                    ),
                },
            },
        };
    }

    public static async toDomainModel(
        dto: DataChannelListDto.DataChannelListPackage
    ): Promise<DataChannelList.DataChannelListPackage> {
        const dataChannels: DataChannelList.DataChannel[] = [];

        for (let dataChannel of dto.Package.DataChannelList.DataChannel) {
            dataChannels.push(await DataChannelExtension.toDomainModel(dataChannel));
        }

        const {
            DataChannelListID,
            DateCreated,
            ShipID,
            Author,
            VersionInformation,
            ...customHeaders
        } = dto.Package.Header;

        return {
            package: {
                header: {
                    dataChannelListId: {
                        id: DataChannelListID.ID,
                        timestamp: DataChannelListID.TimeStamp,
                        version: Version.parse(DataChannelListID.Version),
                    },
                    dateCreated: DateCreated,
                    shipId: ShipId.parse(ShipID),
                    author: Author,
                    versionInformation: VersionInformation
                        ? {
                              namingRule: VersionInformation.NamingRule,
                              namingSchemeVersion:
                                  VersionInformation.NamingSchemeVersion,
                              referenceUrl: VersionInformation.ReferenceURL,
                          }
                        : undefined,
                    customHeaders: customHeaders,
                },
                dataChannelList: {
                    dataChannel: dataChannels,
                },
            },
        };
    }
}

export class DataChannelExtension {
    public static toJsonDto(
        dataChannel: DataChannelList.DataChannel
    ): DataChannelListDto.DataChannel {
        return {
            DataChannelID: {
                LocalID: dataChannel.dataChannelId.localId.verboseMode
                    ? dataChannel.dataChannelId.localId.builder
                          .withoutVerboseMode()
                          .build()
                          .toString()
                    : dataChannel.dataChannelId.localId.toString(),
                ShortID: dataChannel.dataChannelId.shortId,
                NameObject: dataChannel.dataChannelId.nameObject
                    ? {
                          NamingRule: dataChannel.dataChannelId.nameObject.namingRule,
                          ...dataChannel.dataChannelId.nameObject.customProperties,
                      }
                    : undefined,
            },
            Property: {
                DataChannelType: {
                    Type: dataChannel.property.dataChannelType.type,
                    CalculationPeriod:
                        dataChannel.property.dataChannelType.calculationPeriod,
                    UpdateCycle: dataChannel.property.dataChannelType.updateCycle,
                },
                Format: {
                    Type: dataChannel.property.format.type,
                    Restriction: dataChannel.property.format.restriction
                        ? {
                              Enumeration:
                                  dataChannel.property.format.restriction.enumeration,
                              FractionDigits:
                                  dataChannel.property.format.restriction.fractionDigits,
                              Length: dataChannel.property.format.restriction.length,
                              MaxExclusive:
                                  dataChannel.property.format.restriction.maxExclusive,
                              MaxInclusive:
                                  dataChannel.property.format.restriction.maxInclusive,
                              MaxLength:
                                  dataChannel.property.format.restriction.maxLength,
                              MinExclusive:
                                  dataChannel.property.format.restriction.minExclusive,
                              MinInclusive:
                                  dataChannel.property.format.restriction.minInclusive,
                              MinLength:
                                  dataChannel.property.format.restriction.minLength,
                              Pattern: dataChannel.property.format.restriction.pattern,
                              TotalDigits:
                                  dataChannel.property.format.restriction.totalDigits,
                              WhiteSpace: dataChannel.property.format.restriction
                                  .whiteSpace
                                  ? (dataChannel.property.format.restriction
                                        .whiteSpace as unknown as DataChannelListDto.WhiteSpace)
                                  : undefined,
                          }
                        : undefined,
                },
                Range: dataChannel.property.range
                    ? {
                          Low: dataChannel.property.range.low,
                          High: dataChannel.property.range.high,
                      }
                    : undefined,
                Unit: dataChannel.property.unit
                    ? {
                          UnitSymbol: dataChannel.property.unit.unitSymbol,
                          QuantityName: dataChannel.property.unit.quantityName,
                          ...dataChannel.property.unit.customProperties,
                      }
                    : undefined,
                QualityCoding: dataChannel.property.qualityCoding,
                AlertPriority: dataChannel.property.alertPriority,
                Name: dataChannel.property.name,
                Remarks: dataChannel.property.remarks,
                ...dataChannel.property.customProperties,
            },
        };
    }

    public static async toDomainModel(
        dataChannel: DataChannelListDto.DataChannel
    ): Promise<DataChannelList.DataChannel> {
        const localId = await LocalId.parseAsync(
            dataChannel.DataChannelID.LocalID
        );

        const mapNameObject = (): DataChannelList.NameObject | undefined => {
            if (!dataChannel.DataChannelID.NameObject) return undefined;

            const { NamingRule, ...custom } =
                dataChannel.DataChannelID.NameObject;
            return {
                namingRule: NamingRule,
                customProperties: custom,
            };
        };

        const {
            DataChannelType,
            Format,
            Range,
            Unit,
            QualityCoding,
            AlertPriority,
            Name,
            Remarks,
            ...customProperties
        } = dataChannel.Property;

        const mapUnit = (): DataChannelList.Unit | undefined => {
            if (!Unit) return undefined;

            const { UnitSymbol, QuantityName, ...custom } = Unit;
            return {
                unitSymbol: Unit.UnitSymbol,
                quantityName: Unit.QuantityName,
                customProperties: custom,
            };
        };

        return {
            dataChannelId: {
                localId: localId,
                shortId: dataChannel.DataChannelID.ShortID,
                nameObject: mapNameObject(),
            },
            property: {
                dataChannelType: {
                    type: DataChannelType.Type,
                    calculationPeriod: DataChannelType.CalculationPeriod,
                    updateCycle: DataChannelType.UpdateCycle,
                },
                format: {
                    type: Format.Type,
                    restriction: Format.Restriction
                        ? {
                              enumeration: Format.Restriction.Enumeration,
                              fractionDigits: Format.Restriction.FractionDigits,
                              length: Format.Restriction.Length,
                              maxExclusive: Format.Restriction.MaxExclusive,
                              maxInclusive: Format.Restriction.MaxInclusive,
                              maxLength: Format.Restriction?.MaxLength,
                              minExclusive: Format.Restriction.MinExclusive,
                              minInclusive: Format.Restriction.MinInclusive,
                              minLength: Format.Restriction.MinLength,
                              pattern: Format.Restriction.Pattern,
                              totalDigits: Format.Restriction.TotalDigits,
                              whiteSpace: Format.Restriction.WhiteSpace,
                          }
                        : undefined,
                },
                range: Range
                    ? {
                          low: Range.Low,
                          high: Range.High,
                      }
                    : undefined,
                unit: mapUnit(),
                qualityCoding: QualityCoding,
                alertPriority: AlertPriority,
                name: Name,
                remarks: Remarks,
                customProperties: customProperties,
            },
        };
    }
}
