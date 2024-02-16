using Vista.SDK.Transport;
using Domain = Vista.SDK.Experimental.Transport.DataList;

namespace Vista.SDK.Experimental.Transport.Json.DataList;

public static class Extensions
{
    public static DataListPackage ToJsonDto(this Domain.DataListPackage package)
    {
        return new DataListPackage(
            new Package(
                new DataList(
                    package
                        .Package
                        .DataList
                        .Data
                        .Select(
                            c =>
                                new Data(
                                    new DataID(
                                        c.DataId.LocalId.ToString(),
                                        c.DataId.NameObject is null
                                            ? null
                                            : new NameObject(c.DataId.NameObject.NamingRule)
                                            {
                                                CustomProperties = c.DataId
                                                    .NameObject
                                                    .CustomProperties
                                                    .CopyProperties(),
                                            },
                                        c.DataId.ShortId
                                    ),
                                    new Property(
                                        c.Property.AlertPriority,
                                        new DataType(
                                            c.Property.DataType.CalculationPeriod,
                                            c.Property.DataType.Type,
                                            c.Property.DataType.UpdateCycle
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
                                            : new Unit(c.Property.Unit.QuantityName, c.Property.Unit.UnitSymbol)
                                            {
                                                CustomProperties = c.Property.Unit.CustomProperties.CopyProperties(),
                                            }
                                    )
                                    {
                                        CustomProperties = c.Property.CustomProperties.CopyProperties(),
                                    }
                                )
                        )
                        .ToArray()
                ),
                new Header(
                    package.Package.Header.AssetId.ToString(),
                    package.Package.Header.Author,
                    new ConfigurationReference(
                        package.Package.Header.DataListId.Id,
                        package.Package.Header.DataListId.TimeStamp,
                        package.Package.Header.DataListId.Version
                    ),
                    package.Package.Header.DateCreated,
                    package.Package.Header.VersionInformation is null
                        ? null
                        : new VersionInformation(
                            package.Package.Header.VersionInformation.NamingRule,
                            package.Package.Header.VersionInformation.NamingSchemeVersion,
                            package.Package.Header.VersionInformation.ReferenceUrl
                        )
                )
                {
                    CustomHeaders = package.Package.Header.CustomHeaders.CopyProperties(),
                }
            )
        );
    }

    public static Domain.DataListPackage ToDomainModel(this DataListPackage package)
    {
        var p = package.Package;
        return new Domain.DataListPackage(
            new Domain.Package(
                new Domain.Header(
                    AssetIdentifier.Parse(p.Header.AssetId),
                    new Domain.ConfigurationReference(
                        p.Header.DataListID.ID,
                        p.Header.DataListID.Version,
                        p.Header.DataListID.TimeStamp
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
                    p.Header.CustomHeaders.CopyProperties()
                ),
                new Domain.DataList(
                    p.DataList
                        .Data
                        .Select(c =>
                        {
                            if (c.DataID.LocalID is null)
                                throw new InvalidOperationException("DataPackage local ID was null");
                            var id = DataId.Parse(c.DataID.LocalID);
                            return new Domain.Data(
                                new Domain.DataId(
                                    id.Match<ILocalId>(
                                        l => l,
                                        p => p,
                                        s =>
                                            throw new InvalidOperationException(
                                                $"DataPackage local ID not a valid LocalId: {s}"
                                            )
                                    ),
                                    c.DataID.ShortID,
                                    c.DataID.NameObject is null
                                        ? null
                                        : new Domain.NameObject(
                                            c.DataID.NameObject.NamingRule,
                                            c.DataID.NameObject.CustomProperties.CopyProperties()
                                        )
                                ),
                                new Domain.Property(
                                    new Domain.DataType(
                                        c.Property.DataType.Type,
                                        c.Property.DataType.UpdateCycle,
                                        c.Property.DataType.CalculationPeriod
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
                                        : new Domain.Range(c.Property.Range.High, c.Property.Range.Low),
                                    c.Property.Unit is null
                                        ? null
                                        : new Domain.Unit(
                                            c.Property.Unit.UnitSymbol,
                                            c.Property.Unit.QuantityName,
                                            c.Property.Unit.CustomProperties.CopyProperties()
                                        ),
                                    c.Property.QualityCoding,
                                    c.Property.AlertPriority,
                                    c.Property.Name,
                                    c.Property.Remarks,
                                    c.Property.CustomProperties.CopyProperties()
                                )
                            );
                        })
                        .ToList()
                )
            )
        );
    }
}
