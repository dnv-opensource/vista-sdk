using Domain = Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Transport.Json.DataChannel;

public static class Extensions
{
    public static DataChannelListPackage ToJsonDto(this Domain.DataChannelListPackage package)
    {
        return new DataChannelListPackage(
            new Package(
                new DataChannelList(
                    package.Package.DataChannelList.DataChannel
                        .Select(
                            c =>
                                new DataChannel(
                                    new DataChannelID(
                                        c.DataChannelId.LocalId.ToString(),
                                        c.DataChannelId.NameObject is null
                                          ? null
                                          : new NameObject(c.DataChannelId.NameObject.NamingRule)
                                            {
                                                AdditionalProperties =
                                                    c.DataChannelId.NameObject.CustomProperties.CopyProperties(),
                                            },
                                        c.DataChannelId.ShortId
                                    ),
                                    new Property(
                                        c.Property.AlertPriority,
                                        new DataChannelType(
                                            c.Property.DataChannelType.CalculationPeriod,
                                            c.Property.DataChannelType.Type,
                                            c.Property.DataChannelType.UpdateCycle
                                        ),
                                        new Format(
                                            c.Property.Format.Restriction is null
                                              ? null
                                              : new Restriction(
                                                    c.Property.Format.Restriction.Enumeration,
                                                    c.Property.Format.Restriction.FractionDigits,
                                                    c.Property.Format.Restriction.Length,
                                                    c.Property.Format.Restriction.MaxExclusive,
                                                    c.Property.Format.Restriction.MaxInclusive,
                                                    c.Property.Format.Restriction.MaxLength,
                                                    c.Property.Format.Restriction.MinExclusive,
                                                    c.Property.Format.Restriction.MinInclusive,
                                                    c.Property.Format.Restriction.MinLength,
                                                    c.Property.Format.Restriction.Pattern,
                                                    c.Property.Format.Restriction.TotalDigits,
                                                    (RestrictionWhiteSpace?)c.Property.Format.Restriction.WhiteSpace
                                                ),
                                            c.Property.Format.Type
                                        ),
                                        c.Property.Name,
                                        c.Property.QualityCoding,
                                        c.Property.Range is null
                                          ? null
                                          : new Range(c.Property.Range.High, c.Property.Range.Low),
                                        c.Property.Remarks,
                                        c.Property.Unit is null
                                          ? null
                                          : new Unit(
                                                c.Property.Unit.QuantityName,
                                                c.Property.Unit.UnitSymbol
                                            )
                                            {
                                                AdditionalProperties =
                                                    c.Property.Unit.CustomProperties.CopyProperties(),
                                            }
                                    )
                                    {
                                        AdditionalProperties =
                                            c.Property.CustomProperties.CopyProperties(),
                                    }
                                )
                        )
                        .ToArray()
                ),
                new Header(
                    package.Package.Header.Author,
                    new ConfigurationReference(
                        package.Package.Header.DataChannelListId.Id,
                        package.Package.Header.DataChannelListId.TimeStamp,
                        package.Package.Header.DataChannelListId.Version
                    ),
                    package.Package.Header.DateCreated,
                    package.Package.Header.ShipId.ToString(),
                    package.Package.Header.VersionInformation is null
                      ? null
                      : new VersionInformation(
                            package.Package.Header.VersionInformation.NamingRule,
                            package.Package.Header.VersionInformation.NamingSchemeVersion,
                            package.Package.Header.VersionInformation.ReferenceUrl
                        )
                )
                {
                    AdditionalProperties = package.Package.Header.CustomHeaders.CopyProperties(),
                }
            )
        );
    }

    public static Domain.DataChannelListPackage ToDomainModel(this DataChannelListPackage package)
    {
        var p = package.Package;
        return new Domain.DataChannelListPackage(
            new Domain.Package(
                new Domain.Header(
                    ShipId.Parse(p.Header.ShipID),
                    new Domain.ConfigurationReference(
                        p.Header.DataChannelListID.ID,
                        p.Header.DataChannelListID.Version,
                        p.Header.DataChannelListID.TimeStamp
                    ),
                    p.Header.VersionInformation is null
                      ? null
                      : new Domain.VersionInformation(
                            p.Header.VersionInformation.NamingRule,
                            p.Header.VersionInformation.NamingSchemeVersion,
                            p.Header.VersionInformation.ReferenceURL
                        ),
                    p.Header.Author,
                    p.Header.DateCreated,
                    p.Header.AdditionalProperties.CopyProperties()
                ),
                new Domain.DataChannelList(
                    p.DataChannelList.DataChannel
                        .Select(
                            c =>
                                new Domain.DataChannel(
                                    new Domain.DataChannelId(
                                        c.DataChannelID.LocalID is null
                                          ? throw new InvalidOperationException(
                                                "DataChannelPackage local ID was null"
                                            )
                                          : LocalId.Parse(c.DataChannelID.LocalID),
                                        c.DataChannelID.ShortID,
                                        c.DataChannelID.NameObject is null
                                          ? null
                                          : new Domain.NameObject(
                                                c.DataChannelID.NameObject.NamingRule,
                                                c.DataChannelID.NameObject.AdditionalProperties.CopyProperties()
                                            )
                                    ),
                                    new Domain.Property(
                                        new Domain.DataChannelType(
                                            c.Property.DataChannelType.Type,
                                            c.Property.DataChannelType.UpdateCycle,
                                            c.Property.DataChannelType.CalculationPeriod
                                        ),
                                        new Domain.Format(
                                            c.Property.Format.Type,
                                            c.Property.Format.Restriction is null
                                              ? null
                                              : new Domain.Restriction(
                                                    c.Property.Format.Restriction.Enumeration?.ToList(),
                                                    c.Property.Format.Restriction.FractionDigits,
                                                    c.Property.Format.Restriction.Length,
                                                    c.Property.Format.Restriction.MaxExclusive,
                                                    c.Property.Format.Restriction.MaxInclusive,
                                                    c.Property.Format.Restriction.MaxLength,
                                                    c.Property.Format.Restriction.MinExclusive,
                                                    c.Property.Format.Restriction.MinInclusive,
                                                    c.Property.Format.Restriction.MinLength,
                                                    c.Property.Format.Restriction.Pattern,
                                                    c.Property.Format.Restriction.TotalDigits,
                                                    (Domain.WhiteSpace?)c.Property.Format.Restriction.WhiteSpace
                                                )
                                        ),
                                        c.Property.Range is null
                                          ? null
                                          : new Domain.Range(
                                                c.Property.Range.High,
                                                c.Property.Range.Low
                                            ),
                                        c.Property.Unit is null
                                          ? null
                                          : new Domain.Unit(
                                                c.Property.Unit.UnitSymbol,
                                                c.Property.Unit.QuantityName,
                                                c.Property.Unit.AdditionalProperties.CopyProperties()
                                            ),
                                        c.Property.QualityCoding,
                                        c.Property.AlertPriority,
                                        c.Property.Name,
                                        c.Property.Remarks,
                                        c.Property.AdditionalProperties.CopyProperties()
                                    )
                                )
                        )
                        .ToList()
                )
            )
        );
    }
}
