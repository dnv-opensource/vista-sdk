namespace Vista.SDK.Transport.Xml.TimeSeriesData;

// NOTE: Generated code may require at least .NET Framework 4.5 or .NET Core/Standard 2.0.
/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(
    AnonymousType = true,
    Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT",
    TypeName = "Package"
)]
[System.Xml.Serialization.XmlRootAttribute(
    Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT",
    ElementName = "Package",
    IsNullable = false
)]
public partial class TimeSeriesDataPackage
{
    private PackageHeader? headerField;

    private PackageTimeSeriesData[]? timeSeriesDataField;

    /// <remarks/>
    public PackageHeader? Header
    {
        get { return this.headerField; }
        set { this.headerField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("TimeSeriesData")]
    public PackageTimeSeriesData[]? TimeSeriesData
    {
        get { return this.timeSeriesDataField; }
        set { this.timeSeriesDataField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageHeader
{
    private string? shipIDField;

    private PackageHeaderTimeSpan? timeSpanField;

    private System.DateTime dateCreatedField;

    private System.DateTime dateModifiedField;

    private string? authorField;

    private PackageHeaderSystemConfiguration[]? systemConfigurationField;

    /// <remarks/>
    public string? ShipID
    {
        get { return this.shipIDField; }
        set { this.shipIDField = value; }
    }

    /// <remarks/>
    public PackageHeaderTimeSpan? TimeSpan
    {
        get { return this.timeSpanField; }
        set { this.timeSpanField = value; }
    }

    /// <remarks/>
    public System.DateTime DateCreated
    {
        get { return this.dateCreatedField; }
        set { this.dateCreatedField = value; }
    }

    /// <remarks/>
    public System.DateTime DateModified
    {
        get { return this.dateModifiedField; }
        set { this.dateModifiedField = value; }
    }

    /// <remarks/>
    public string? Author
    {
        get { return this.authorField; }
        set { this.authorField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("SystemConfiguration")]
    public PackageHeaderSystemConfiguration[]? SystemConfiguration
    {
        get { return this.systemConfigurationField; }
        set { this.systemConfigurationField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageHeaderTimeSpan
{
    private System.DateTime startField;

    private System.DateTime endField;

    /// <remarks/>
    public System.DateTime Start
    {
        get { return this.startField; }
        set { this.startField = value; }
    }

    /// <remarks/>
    public System.DateTime End
    {
        get { return this.endField; }
        set { this.endField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageHeaderSystemConfiguration
{
    private string? idField;

    private System.DateTime timeStampField;

    /// <remarks/>
    public string? ID
    {
        get { return this.idField; }
        set { this.idField = value; }
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
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesData
{
    private PackageTimeSeriesDataDataConfiguration? dataConfigurationField;

    private PackageTimeSeriesDataTabularData[]? tabularDataField;

    private PackageTimeSeriesDataEventData? eventDataField;

    /// <remarks/>
    public PackageTimeSeriesDataDataConfiguration? DataConfiguration
    {
        get { return this.dataConfigurationField; }
        set { this.dataConfigurationField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("TabularData")]
    public PackageTimeSeriesDataTabularData[]? TabularData
    {
        get { return this.tabularDataField; }
        set { this.tabularDataField = value; }
    }

    /// <remarks/>
    public PackageTimeSeriesDataEventData? EventData
    {
        get { return this.eventDataField; }
        set { this.eventDataField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataDataConfiguration
{
    private string? idField;

    private System.DateTime timeStampField;

    /// <remarks/>
    public string? ID
    {
        get { return this.idField; }
        set { this.idField = value; }
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
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataTabularData
{
    private string? numberOfDataSetField;

    private string? numberOfDataChannelField;

    private PackageTimeSeriesDataTabularDataDataChannelID[]? dataChannelIDField;

    private PackageTimeSeriesDataTabularDataDataSet[]? dataSetField;

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? NumberOfDataSet
    {
        get { return this.numberOfDataSetField; }
        set { this.numberOfDataSetField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? NumberOfDataChannel
    {
        get { return this.numberOfDataChannelField; }
        set { this.numberOfDataChannelField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("DataChannelID")]
    public PackageTimeSeriesDataTabularDataDataChannelID[]? DataChannelID
    {
        get { return this.dataChannelIDField; }
        set { this.dataChannelIDField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("DataSet")]
    public PackageTimeSeriesDataTabularDataDataSet[]? DataSet
    {
        get { return this.dataSetField; }
        set { this.dataSetField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataTabularDataDataChannelID
{
    private string? idField;

    private string? valueField;

    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute(DataType = "integer")]
    public string? id
    {
        get { return this.idField; }
        set { this.idField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlTextAttribute()]
    public string? Value
    {
        get { return this.valueField; }
        set { this.valueField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataTabularDataDataSet
{
    private PackageTimeSeriesDataTabularDataDataSetValue[]? valueField;

    private System.DateTime timeStampField;

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("Value")]
    public PackageTimeSeriesDataTabularDataDataSetValue[]? Value
    {
        get { return this.valueField; }
        set { this.valueField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public System.DateTime timeStamp
    {
        get { return this.timeStampField; }
        set { this.timeStampField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataTabularDataDataSetValue
{
    private string? refField;

    private string? qualityField;

    private string? valueField;

    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute(DataType = "integer")]
    public string? @ref
    {
        get { return this.refField; }
        set { this.refField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public string? quality
    {
        get { return this.qualityField; }
        set { this.qualityField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlTextAttribute()]
    public string? Value
    {
        get { return this.valueField; }
        set { this.valueField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataEventData
{
    private string? numberOfDataSetField;

    private PackageTimeSeriesDataEventDataDataSet[]? dataSetField;

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute(DataType = "integer")]
    public string? NumberOfDataSet
    {
        get { return this.numberOfDataSetField; }
        set { this.numberOfDataSetField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("DataSet")]
    public PackageTimeSeriesDataEventDataDataSet[]? DataSet
    {
        get { return this.dataSetField; }
        set { this.dataSetField = value; }
    }
}

/// <remarks/>
[System.SerializableAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType = true, Namespace = "urn:ISO19848:SHIP_DATA_TRANSPORT")]
public partial class PackageTimeSeriesDataEventDataDataSet
{
    private string? dataChannelIDField;

    private string? valueField;

    private System.DateTime timeStampField;

    private string? qualityField;

    /// <remarks/>
    public string? DataChannelID
    {
        get { return this.dataChannelIDField; }
        set { this.dataChannelIDField = value; }
    }

    /// <remarks/>
    public string? Value
    {
        get { return this.valueField; }
        set { this.valueField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public System.DateTime timeStamp
    {
        get { return this.timeStampField; }
        set { this.timeStampField = value; }
    }

    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public string? quality
    {
        get { return this.qualityField; }
        set { this.qualityField = value; }
    }
}
