namespace Vista.SDK.Transport.Xml.DataChannel;

// NOTE: Generated code may require at least .NET Framework 4.5 or .NET Core/Standard 2.0.
/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(
    AnonymousType = true,
    Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION",
    TypeName = "Package"
)]
[System.Xml.Serialization.XmlRootAttribute(
    Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION",
    ElementName = "Package",
    IsNullable = false
)]
public partial class DataChannelListPackage
{
    private PackageHeader? headerField;

    private PackageDataChannel[]? dataChannelListField;

    /// <remarks/>
    public PackageHeader? Header
    {
        get { return this.headerField; }
        set { this.headerField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlArrayItemAttribute("DataChannel", IsNullable = false)]
    public PackageDataChannel[]? DataChannelList
    {
        get { return this.dataChannelListField; }
        set { this.dataChannelListField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageHeader
{
    private string? shipIDField;

    private PackageHeaderDataChannelListID? dataChannelListIDField;

    private PackageHeaderVersionInformation[]? versionInformationField;

    private string? authorField;

    private System.DateTime dateCreatedField;

    /// <remarks/>
    public string? ShipID
    {
        get { return this.shipIDField; }
        set { this.shipIDField = value; }
    }

    /// <remarks/>
    public PackageHeaderDataChannelListID? DataChannelListID
    {
        get { return this.dataChannelListIDField; }
        set { this.dataChannelListIDField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("VersionInformation")]
    public PackageHeaderVersionInformation[]? VersionInformation
    {
        get { return this.versionInformationField; }
        set { this.versionInformationField = value; }
    }

    /// <remarks/>
    public string? Author
    {
        get { return this.authorField; }
        set { this.authorField = value; }
    }

    /// <remarks/>
    public System.DateTime DateCreated
    {
        get { return this.dateCreatedField; }
        set { this.dateCreatedField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageHeaderDataChannelListID
{
    private string? idField;

    private string? versionField;

    private System.DateTime timeStampField;

    /// <remarks/>
    public string? ID
    {
        get { return this.idField; }
        set { this.idField = value; }
    }

    /// <remarks/>
    public string? Version
    {
        get { return this.versionField; }
        set { this.versionField = value; }
    }

    /// <remarks/>
    public System.DateTime TimeStamp
    {
        get { return this.timeStampField; }
        set { this.timeStampField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageHeaderVersionInformation
{
    private string? namingRuleField;

    private string? namingSchemeVersionField;

    private string? referenceURLField;

    /// <remarks/>
    public string? NamingRule
    {
        get { return this.namingRuleField; }
        set { this.namingRuleField = value; }
    }

    /// <remarks/>
    public string? NamingSchemeVersion
    {
        get { return this.namingSchemeVersionField; }
        set { this.namingSchemeVersionField = value; }
    }

    /// <remarks/>
    public string? ReferenceURL
    {
        get { return this.referenceURLField; }
        set { this.referenceURLField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannel
{
    private PackageDataChannelDataChannelID? dataChannelIDField;

    private PackageDataChannelProperty? propertyField;

    /// <remarks/>
    public PackageDataChannelDataChannelID? DataChannelID
    {
        get { return this.dataChannelIDField; }
        set { this.dataChannelIDField = value; }
    }

    /// <remarks/>
    public PackageDataChannelProperty? Property
    {
        get { return this.propertyField; }
        set { this.propertyField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelDataChannelID
{
    private string? localIDField;

    private string? shortIDField;

    private PackageDataChannelDataChannelIDNameObject? nameObjectField;

    /// <remarks/>
    public string? LocalID
    {
        get { return this.localIDField; }
        set { this.localIDField = value; }
    }

    /// <remarks/>
    public string? ShortID
    {
        get { return this.shortIDField; }
        set { this.shortIDField = value; }
    }

    /// <remarks/>
    public PackageDataChannelDataChannelIDNameObject? NameObject
    {
        get { return this.nameObjectField; }
        set { this.nameObjectField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelDataChannelIDNameObject
{
    private string? namingRuleField;

    /// <remarks/>
    public string? NamingRule
    {
        get { return this.namingRuleField; }
        set { this.namingRuleField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelProperty
{
    private PackageDataChannelPropertyDataChannelType? dataChannelTypeField;

    private PackageDataChannelPropertyFormat? formatField;

    private PackageDataChannelPropertyRange? rangeField;

    private PackageDataChannelPropertyUnit? unitField;

    private string? qualityCodingField;

    private string? alertPriorityField;

    private string? nameField;

    private string? remarksField;

    /// <remarks/>
    public PackageDataChannelPropertyDataChannelType? DataChannelType
    {
        get { return this.dataChannelTypeField; }
        set { this.dataChannelTypeField = value; }
    }

    /// <remarks/>
    public PackageDataChannelPropertyFormat? Format
    {
        get { return this.formatField; }
        set { this.formatField = value; }
    }

    /// <remarks/>
    public PackageDataChannelPropertyRange? Range
    {
        get { return this.rangeField; }
        set { this.rangeField = value; }
    }

    /// <remarks/>
    public PackageDataChannelPropertyUnit? Unit
    {
        get { return this.unitField; }
        set { this.unitField = value; }
    }

    /// <remarks/>
    public string? QualityCoding
    {
        get { return this.qualityCodingField; }
        set { this.qualityCodingField = value; }
    }

    /// <remarks/>
    public string? AlertPriority
    {
        get { return this.alertPriorityField; }
        set { this.alertPriorityField = value; }
    }

    /// <remarks/>
    public string? Name
    {
        get { return this.nameField; }
        set { this.nameField = value; }
    }

    /// <remarks/>
    public string? Remarks
    {
        get { return this.remarksField; }
        set { this.remarksField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelPropertyDataChannelType
{
    private string? typeField;

    private string? updateCycleField;

    private string? calculationPeriodField;

    /// <remarks/>
    public string? Type
    {
        get { return this.typeField; }
        set { this.typeField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? UpdateCycle
    {
        get { return this.updateCycleField; }
        set { this.updateCycleField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? CalculationPeriod
    {
        get { return this.calculationPeriodField; }
        set { this.calculationPeriodField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelPropertyFormat
{
    private string? typeField;

    private PackageDataChannelPropertyFormatRestriction? restrictionField;

    /// <remarks/>
    public string? Type
    {
        get { return this.typeField; }
        set { this.typeField = value; }
    }

    /// <remarks/>
    public PackageDataChannelPropertyFormatRestriction? Restriction
    {
        get { return this.restrictionField; }
        set { this.restrictionField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelPropertyFormatRestriction
{
    private string[]? enumerationField;

    private string? fractionDigitsField;

    private string? lengthField;

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("Enumeration")]
    public string[]? Enumeration
    {
        get { return this.enumerationField; }
        set { this.enumerationField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? FractionDigits
    {
        get { return this.fractionDigitsField; }
        set { this.fractionDigitsField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? Length
    {
        get { return this.lengthField; }
        set { this.lengthField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelPropertyRange
{
    private string? highField;

    private string? lowField;

    /// <remarks/>
    public string? High
    {
        get { return this.highField; }
        set { this.highField = value; }
    }

    /// <remarks/>
    public string? Low
    {
        get { return this.lowField; }
        set { this.lowField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_DEFINITION")]
public partial class PackageDataChannelPropertyUnit
{
    private string? unitSymbolField;

    private string? quantityNameField;

    /// <remarks/>
    public string? UnitSymbol
    {
        get { return this.unitSymbolField; }
        set { this.unitSymbolField = value; }
    }

    /// <remarks/>
    public string? QuantityName
    {
        get { return this.quantityNameField; }
        set { this.quantityNameField = value; }
    }
}
