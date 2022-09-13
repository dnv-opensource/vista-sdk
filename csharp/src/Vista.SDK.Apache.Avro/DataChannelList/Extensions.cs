using Domain = Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Transport.Avro.DataChannel;

public static class Extensions
{
    public static DataChannelListPackage ToAvroDto(this Domain.DataChannelListPackage package)
    {
        var p = package.Package;
        var h = package.Package.Header;
        return new DataChannelListPackage()
        {
            Package = new Package()
            {
                Header = new Header()
                {
                    ShipID = h.ShipId.ToString(),
                    DataChannelListID = new ConfigurationReference()
                    {
                        ID = h.DataChannelListId.Id,
                        TimeStamp = h.DataChannelListId.TimeStamp.DateTime,
                        Version = h.DataChannelListId.Version,
                    },
                    VersionInformation = h.VersionInformation is null
                        ? null
                        : new VersionInformation()
                          {
                              NamingRule = h.VersionInformation.NamingRule,
                              NamingSchemeVersion = h.VersionInformation.NamingSchemeVersion,
                              ReferenceURL = h.VersionInformation.ReferenceUrl,
                          },
                    Author = h.Author,
                    DateCreated = h.DateCreated?.DateTime,
                },
                DataChannelList = new DataChannelList()
                {
                    DataChannel = p.DataChannelList.DataChannel
                        .Select(
                            dc =>
                                new DataChannel()
                                {
                                    DataChannelID = new DataChannelID()
                                    {
                                        LocalID = dc.DataChannelId.LocalId.ToString(),
                                        NameObject = dc.DataChannelId.NameObject is null
                                            ? null
                                            : new NameObject()
                                              {
                                                  NamingRule =
                                                      dc.DataChannelId.NameObject.NamingRule,
                                              },
                                        ShortID = dc.DataChannelId.ShortId,
                                    },
                                    Property = new Property()
                                    {
                                        AlertPriority = dc.Property.AlertPriority,
                                        DataChannelType = new DataChannelType()
                                        {
                                            CalculationPeriod =
                                                dc.Property.DataChannelType.CalculationPeriod,
                                            Type = dc.Property.DataChannelType.Type,
                                            UpdateCycle = dc.Property.DataChannelType.UpdateCycle,
                                        },
                                        Format = new Format()
                                        {
                                            Restriction = dc.Property.Format.Restriction is null
                                                ? null
                                                : new Restriction()
                                                  {
                                                      Enumeration =
                                                          dc.Property.Format.Restriction.Enumeration?.ToList(),
                                                      FractionDigits =
                                                          dc.Property.Format.Restriction.FractionDigits,
                                                      Length =
                                                          dc.Property.Format.Restriction.Length,
                                                      MaxExclusive =
                                                          dc.Property.Format.Restriction.MaxExclusive,
                                                      MaxInclusive =
                                                          dc.Property.Format.Restriction.MaxInclusive,
                                                      MaxLength =
                                                          dc.Property.Format.Restriction.MaxLength,
                                                      MinExclusive =
                                                          dc.Property.Format.Restriction.MinExclusive,
                                                      MinInclusive =
                                                          dc.Property.Format.Restriction.MinInclusive,
                                                      MinLength =
                                                          dc.Property.Format.Restriction.MinLength,
                                                      Pattern =
                                                          dc.Property.Format.Restriction.Pattern,
                                                      TotalDigits =
                                                          dc.Property.Format.Restriction.TotalDigits,
                                                      WhiteSpace =
                                                          (WhiteSpace?)dc.Property.Format.Restriction.WhiteSpace,
                                                  },
                                            Type = dc.Property.Format.Type,
                                        },
                                        Name = dc.Property.Name,
                                        QualityCoding = dc.Property.QualityCoding,
                                        Range = dc.Property.Range is null
                                            ? null
                                            : new Range()
                                              {
                                                  High = dc.Property.Range.High,
                                                  Low = dc.Property.Range.Low,
                                              },
                                        Remarks = dc.Property.Remarks,
                                        Unit = dc.Property.Unit is null
                                            ? null
                                            : new Unit()
                                              {
                                                  QuantityName = dc.Property.Unit.QuantityName,
                                                  UnitSymbol = dc.Property.Unit.UnitSymbol,
                                              },
                                    },
                                }
                        )
                        .ToList(),
                },
            },
        };
    }

    public static Transport.DataChannel.DataChannelListPackage ToDomainModel(
        this DataChannelListPackage package
    )
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
                    // p.Header.AdditionalProperties.CopyProperties()
                    new Dictionary<string, object>()
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
                                                // c.DataChannelID.NameObject.AdditionalProperties.CopyProperties()
                                                new Dictionary<string, object>()
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
                                                // c.Property.Unit.AdditionalProperties.CopyProperties()
                                                new Dictionary<string, object>()
                                            ),
                                        c.Property.QualityCoding,
                                        c.Property.AlertPriority,
                                        c.Property.Name,
                                        c.Property.Remarks,
                                        // c.Property.AdditionalProperties.CopyProperties()
                                        new Dictionary<string, object>()
                                    )
                                )
                        )
                        .ToList()
                )
            )
        );
    }
}
