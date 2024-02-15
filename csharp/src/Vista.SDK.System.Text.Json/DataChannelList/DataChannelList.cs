//----------------------
// <auto-generated>
//     Generated using the NJsonSchema v11.0.0.0 (Newtonsoft.Json v13.0.0.0) (http://NJsonSchema.org)
// </auto-generated>
//----------------------


#nullable enable


namespace Vista.SDK.Transport.Json.DataChannel
{
    #pragma warning disable // Disable all warnings

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Package
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Package(DataChannelList @dataChannelList, Header @header)


        {

            this.Header = @header;

            this.DataChannelList = @dataChannelList;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Header")]
        public Header Header { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataChannelList")]
        public DataChannelList DataChannelList { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Header
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Header(string? @author, ConfigurationReference @dataChannelListID, System.DateTimeOffset? @dateCreated, string @shipID, VersionInformation? @versionInformation)


        {

            this.ShipID = @shipID;

            this.DataChannelListID = @dataChannelListID;

            this.VersionInformation = @versionInformation;

            this.Author = @author;

            this.DateCreated = @dateCreated;

        }
        [System.Text.Json.Serialization.JsonPropertyName("ShipID")]
        public string ShipID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataChannelListID")]
        public ConfigurationReference DataChannelListID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("VersionInformation")]
        public VersionInformation? VersionInformation { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Author")]
        public string? Author { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DateCreated")]
        public System.DateTimeOffset? DateCreated { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class VersionInformation
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public VersionInformation(string @namingRule, string @namingSchemeVersion, string? @referenceURL)


        {

            this.NamingRule = @namingRule;

            this.NamingSchemeVersion = @namingSchemeVersion;

            this.ReferenceURL = @referenceURL;

        }
        [System.Text.Json.Serialization.JsonPropertyName("NamingRule")]
        public string NamingRule { get; }


        [System.Text.Json.Serialization.JsonPropertyName("NamingSchemeVersion")]
        public string NamingSchemeVersion { get; }


        [System.Text.Json.Serialization.JsonPropertyName("ReferenceURL")]
        public string? ReferenceURL { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataChannelList
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataChannelList(System.Collections.Generic.IReadOnlyList<DataChannel> @dataChannel)


        {

            this.DataChannel = @dataChannel;

        }
        [System.Text.Json.Serialization.JsonPropertyName("DataChannel")]
        public System.Collections.Generic.IReadOnlyList<DataChannel> DataChannel { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataChannel
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataChannel(DataChannelID @dataChannelID, Property @property)


        {

            this.DataChannelID = @dataChannelID;

            this.Property = @property;

        }
        [System.Text.Json.Serialization.JsonPropertyName("DataChannelID")]
        public DataChannelID DataChannelID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Property")]
        public Property Property { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataChannelID
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataChannelID(string @localID, NameObject? @nameObject, string? @shortID)


        {

            this.LocalID = @localID;

            this.ShortID = @shortID;

            this.NameObject = @nameObject;

        }
        [System.Text.Json.Serialization.JsonPropertyName("LocalID")]
        public string LocalID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("ShortID")]
        public string? ShortID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("NameObject")]
        public NameObject? NameObject { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class NameObject
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public NameObject(string @namingRule)


        {

            this.NamingRule = @namingRule;

        }
        [System.Text.Json.Serialization.JsonPropertyName("NamingRule")]
        public string NamingRule { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Property
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Property(string? @alertPriority, DataChannelType @dataChannelType, Format @format, string? @name, string? @qualityCoding, Range? @range, string? @remarks, Unit? @unit)


        {

            this.DataChannelType = @dataChannelType;

            this.Format = @format;

            this.Range = @range;

            this.Unit = @unit;

            this.QualityCoding = @qualityCoding;

            this.AlertPriority = @alertPriority;

            this.Name = @name;

            this.Remarks = @remarks;

        }
        [System.Text.Json.Serialization.JsonPropertyName("DataChannelType")]
        public DataChannelType DataChannelType { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Format")]
        public Format Format { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Range")]
        public Range? Range { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Unit")]
        public Unit? Unit { get; }


        [System.Text.Json.Serialization.JsonPropertyName("QualityCoding")]
        public string? QualityCoding { get; }


        [System.Text.Json.Serialization.JsonPropertyName("AlertPriority")]
        public string? AlertPriority { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Name")]
        public string? Name { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Remarks")]
        public string? Remarks { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataChannelType
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataChannelType(double? @calculationPeriod, string @type, double? @updateCycle)


        {

            this.Type = @type;

            this.UpdateCycle = @updateCycle;

            this.CalculationPeriod = @calculationPeriod;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Type")]
        public string Type { get; }


        [System.Text.Json.Serialization.JsonPropertyName("UpdateCycle")]
        public double? UpdateCycle { get; }


        [System.Text.Json.Serialization.JsonPropertyName("CalculationPeriod")]
        public double? CalculationPeriod { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Format
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Format(Restriction? @restriction, string @type)


        {

            this.Type = @type;

            this.Restriction = @restriction;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Type")]
        public string Type { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Restriction")]
        public Restriction? Restriction { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Range
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Range(double @high, double @low)


        {

            this.High = @high;

            this.Low = @low;

        }
        [System.Text.Json.Serialization.JsonPropertyName("High")]
        public double High { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Low")]
        public double Low { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Unit
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Unit(string? @quantityName, string @unitSymbol)


        {

            this.UnitSymbol = @unitSymbol;

            this.QuantityName = @quantityName;

        }
        [System.Text.Json.Serialization.JsonPropertyName("UnitSymbol")]
        public string UnitSymbol { get; }


        [System.Text.Json.Serialization.JsonPropertyName("QuantityName")]
        public string? QuantityName { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Restriction
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Restriction(System.Collections.Generic.IReadOnlyList<string>? @enumeration, int? @fractionDigits, int? @length, double? @maxExclusive, double? @maxInclusive, int? @maxLength, double? @minExclusive, double? @minInclusive, int? @minLength, string? @pattern, int? @totalDigits, RestrictionWhiteSpace? @whiteSpace)


        {

            this.Enumeration = @enumeration;

            this.FractionDigits = @fractionDigits;

            this.Length = @length;

            this.MaxExclusive = @maxExclusive;

            this.MaxInclusive = @maxInclusive;

            this.MaxLength = @maxLength;

            this.MinExclusive = @minExclusive;

            this.MinInclusive = @minInclusive;

            this.MinLength = @minLength;

            this.Pattern = @pattern;

            this.TotalDigits = @totalDigits;

            this.WhiteSpace = @whiteSpace;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Enumeration")]
        public System.Collections.Generic.IReadOnlyList<string>? Enumeration { get; }


        [System.Text.Json.Serialization.JsonPropertyName("FractionDigits")]
        public int? FractionDigits { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Length")]
        public int? Length { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MaxExclusive")]
        public double? MaxExclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MaxInclusive")]
        public double? MaxInclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MaxLength")]
        public int? MaxLength { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MinExclusive")]
        public double? MinExclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MinInclusive")]
        public double? MinInclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MinLength")]
        public int? MinLength { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Pattern")]
        public string? Pattern { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TotalDigits")]
        public int? TotalDigits { get; }


        [System.Text.Json.Serialization.JsonPropertyName("WhiteSpace")]
        [System.Text.Json.Serialization.JsonConverter(typeof(System.Text.Json.Serialization.JsonStringEnumConverter))]
        public RestrictionWhiteSpace? WhiteSpace { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class ConfigurationReference
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public ConfigurationReference(string @iD, System.DateTimeOffset @timeStamp, string? @version)


        {

            this.ID = @iD;

            this.Version = @version;

            this.TimeStamp = @timeStamp;

        }
        [System.Text.Json.Serialization.JsonPropertyName("ID")]
        public string ID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Version")]
        public string? Version { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TimeStamp")]
        public System.DateTimeOffset TimeStamp { get; }


    }

    /// <summary>
    /// A DataChannelList package for ISO19848
    /// </summary>
    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataChannelListPackage
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataChannelListPackage(Package @package)


        {

            this.Package = @package;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Package")]
        public Package Package { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "11.0.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public enum RestrictionWhiteSpace
    {

        [System.Runtime.Serialization.EnumMember(Value = @"Preserve")]
        Preserve = 0,


        [System.Runtime.Serialization.EnumMember(Value = @"Replace")]
        Replace = 1,


        [System.Runtime.Serialization.EnumMember(Value = @"Collapse")]
        Collapse = 2,


    }
}