using Domain = Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Transport.Json.DataChannel;

public static class Extensions
{
    public static DataChannelListPackage ToJsonDto(this Domain.DataChannelListPackage package)
    {
        return new DataChannelListPackage(
            new Package(
                new DataChannelList(
                    package
                        .DataChannelList
                        .Select(
                            c =>
                                new DataChannel(
                                    new DataChannelID(
                                        c.DataChannelId.LocalId.VerboseMode
                                            ? c.DataChannelId.LocalId.Builder.WithVerboseMode(false).Build().ToString()
                                            : c.DataChannelId.LocalId.Builder.ToString(),
                                        c.DataChannelId.NameObject is null
                                            ? null
                                            : new NameObject(c.DataChannelId.NameObject.NamingRule)
                                            {
                                                CustomProperties = c.DataChannelId
                                                    .NameObject
                                                    .CustomNameObjects
                                                    ?.CopyProperties(),
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
                                                    (int?)c.Property.Format.Restriction.FractionDigits,
                                                    (int?)c.Property.Format.Restriction.Length,
                                                    c.Property.Format.Restriction.MaxExclusive,
                                                    c.Property.Format.Restriction.MaxInclusive,
                                                    (int?)c.Property.Format.Restriction.MaxLength,
                                                    c.Property.Format.Restriction.MinExclusive,
                                                    c.Property.Format.Restriction.MinInclusive,
                                                    (int?)c.Property.Format.Restriction.MinLength,
                                                    c.Property.Format.Restriction.Pattern,
                                                    (int?)c.Property.Format.Restriction.TotalDigits,
                                                    (RestrictionWhiteSpace?)c.Property.Format.Restriction.WhiteSpace
                                                ),
                                            c.Property.Format.Type
                                        ),
                                        c.Property.Name,
                                        c.Property.QualityCoding,
                                        c.Property.Range is null
                                            ? null
                                            : new Range((double)c.Property.Range.High, c.Property.Range.Low),
                                        c.Property.Remarks,
                                        c.Property.Unit is null
                                            ? null
                                            : new Unit(c.Property.Unit.QuantityName, c.Property.Unit.UnitSymbol)
                                            {
                                                CustomProperties = c.Property.Unit.CustomElements?.CopyProperties()
                                            }
                                    )
                                    {
                                        CustomProperties = c.Property.CustomProperties?.CopyProperties()
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
                    CustomHeaders = package.Package.Header.CustomHeaders?.CopyProperties()
                }
            )
        );
    }

    public static Domain.DataChannelListPackage ToDomainModel(this DataChannelListPackage package)
    {
        var p = package.Package;
        return new Domain.DataChannelListPackage
        {
            Package = new Domain.Package
            {
                Header = new Domain.Header
                {
                    ShipId = ShipId.Parse(p.Header.ShipID),
                    DataChannelListId = new Domain.ConfigurationReference
                    {
                        Id = p.Header.DataChannelListID.ID,
                        Version = p.Header.DataChannelListID.Version,
                        TimeStamp = p.Header.DataChannelListID.TimeStamp
                    },
                    VersionInformation = p.Header.VersionInformation is null
                        ? null
                        : new Domain.VersionInformation
                        {
                            NamingRule = p.Header.VersionInformation.NamingRule,
                            NamingSchemeVersion = p.Header.VersionInformation.NamingSchemeVersion,
                            ReferenceUrl = p.Header.VersionInformation.ReferenceURL
                        },
                    Author = p.Header.Author,
                    DateCreated = p.Header.DateCreated,
                    CustomHeaders = p.Header.CustomHeaders?.CopyProperties()
                },
                DataChannelList = new Domain.DataChannelList
                {
                    p.DataChannelList
                        .DataChannel
                        .Select(
                            c =>
                                new Domain.DataChannel
                                {
                                    DataChannelId = new Domain.DataChannelId
                                    {
                                        LocalId = LocalId.Parse(c.DataChannelID.LocalID),
                                        ShortId = c.DataChannelID.ShortID,
                                        NameObject = c.DataChannelID.NameObject is null
                                            ? null
                                            : new Domain.NameObject
                                            {
                                                NamingRule = c.DataChannelID.NameObject.NamingRule,
                                                CustomNameObjects = c.DataChannelID
                                                    .NameObject
                                                    .CustomProperties
                                                    ?.CopyProperties()
                                            }
                                    },
                                    Property = new Domain.Property
                                    {
                                        DataChannelType = new Domain.DataChannelType
                                        {
                                            Type = c.Property.DataChannelType.Type,
                                            UpdateCycle = c.Property.DataChannelType.UpdateCycle,
                                            CalculationPeriod = c.Property.DataChannelType.CalculationPeriod
                                        },
                                        Format = new Domain.Format
                                        {
                                            Type = c.Property.Format.Type,
                                            Restriction = c.Property.Format.Restriction is null
                                                ? null
                                                : new Domain.Restriction
                                                {
                                                    Enumeration = c.Property.Format.Restriction.Enumeration?.ToList(),
                                                    FractionDigits = (uint?)
                                                        c.Property.Format.Restriction.FractionDigits,
                                                    Length = (uint?)c.Property.Format.Restriction.Length,
                                                    MaxExclusive = c.Property.Format.Restriction.MaxExclusive,
                                                    MaxInclusive = c.Property.Format.Restriction.MaxInclusive,
                                                    MaxLength = (uint?)c.Property.Format.Restriction.MaxLength,
                                                    MinExclusive = c.Property.Format.Restriction.MinExclusive,
                                                    MinInclusive = c.Property.Format.Restriction.MinInclusive,
                                                    MinLength = (uint?)c.Property.Format.Restriction.MinLength,
                                                    Pattern = c.Property.Format.Restriction.Pattern,
                                                    TotalDigits = (uint?)c.Property.Format.Restriction.TotalDigits,
                                                    WhiteSpace = (Domain.WhiteSpace?)
                                                        c.Property.Format.Restriction.WhiteSpace
                                                }
                                        },
                                        Range = c.Property.Range is null
                                            ? null
                                            : new Domain.Range
                                            {
                                                Low = c.Property.Range.Low,
                                                High = c.Property.Range.High
                                            },
                                        Unit = c.Property.Unit is not null
                                            ? new Domain.Unit
                                            {
                                                UnitSymbol = c.Property.Unit.UnitSymbol,
                                                QuantityName = c.Property.Unit.QuantityName,
                                                CustomElements = c.Property.Unit.CustomProperties?.CopyProperties()
                                            }
                                            : null,
                                        QualityCoding = c.Property.QualityCoding,
                                        AlertPriority = c.Property.AlertPriority,
                                        Name = c.Property.Name,
                                        Remarks = c.Property.Remarks,
                                        CustomProperties = c.Property.CustomProperties?.CopyProperties()
                                    }
                                }
                        )
                        .ToList()
                }
            }
        };
    }
}
