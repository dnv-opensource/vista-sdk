//----------------------
// <auto-generated>
//     Generated using the NJsonSchema v10.6.10.0 (Newtonsoft.Json v13.0.0.0) (http://NJsonSchema.org)
// </auto-generated>
//----------------------


#nullable enable


namespace Vista.SDK.Transport.Json.TimeSeriesData
{
    #pragma warning disable // Disable all warnings

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Package
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Package(Header? @header, System.Collections.Generic.IReadOnlyList<TimeSeriesData> @timeSeriesData)


        {

            this.Header = @header;

            this.TimeSeriesData = @timeSeriesData;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Header")]
        public Header? Header { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TimeSeriesData")]
        public System.Collections.Generic.IReadOnlyList<TimeSeriesData> TimeSeriesData { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Header
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Header(string? @author, System.DateTimeOffset? @dateCreated, System.DateTimeOffset? @dateModified, string @shipID, System.Collections.Generic.IReadOnlyList<ConfigurationReference>? @systemConfiguration, TimeSpan? @timeSpan)


        {

            this.ShipID = @shipID;

            this.TimeSpan = @timeSpan;

            this.DateCreated = @dateCreated;

            this.DateModified = @dateModified;

            this.Author = @author;

            this.SystemConfiguration = @systemConfiguration;

        }
        [System.Text.Json.Serialization.JsonPropertyName("ShipID")]
        public string ShipID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TimeSpan")]
        public TimeSpan? TimeSpan { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DateCreated")]
        public System.DateTimeOffset? DateCreated { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DateModified")]
        public System.DateTimeOffset? DateModified { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Author")]
        public string? Author { get; }


        [System.Text.Json.Serialization.JsonPropertyName("SystemConfiguration")]
        public System.Collections.Generic.IReadOnlyList<ConfigurationReference>? SystemConfiguration { get; }



        private System.Collections.Generic.IDictionary<string, object> _additionalProperties = new System.Collections.Generic.Dictionary<string, object>();

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties; }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class TimeSpan
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public TimeSpan(System.DateTimeOffset @end, System.DateTimeOffset @start)


        {

            this.Start = @start;

            this.End = @end;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Start")]
        public System.DateTimeOffset Start { get; }


        [System.Text.Json.Serialization.JsonPropertyName("End")]
        public System.DateTimeOffset End { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class ConfigurationReference
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public ConfigurationReference(string @iD, System.DateTimeOffset @timeStamp)


        {

            this.ID = @iD;

            this.TimeStamp = @timeStamp;

        }
        [System.Text.Json.Serialization.JsonPropertyName("ID")]
        public string ID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TimeStamp")]
        public System.DateTimeOffset TimeStamp { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class TimeSeriesData
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public TimeSeriesData(ConfigurationReference? @dataConfiguration, EventData? @eventData, System.Collections.Generic.IReadOnlyList<TabularData>? @tabularData)


        {

            this.DataConfiguration = @dataConfiguration;

            this.TabularData = @tabularData;

            this.EventData = @eventData;

        }
        [System.Text.Json.Serialization.JsonPropertyName("DataConfiguration")]
        public ConfigurationReference? DataConfiguration { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TabularData")]
        public System.Collections.Generic.IReadOnlyList<TabularData>? TabularData { get; }


        [System.Text.Json.Serialization.JsonPropertyName("EventData")]
        public EventData? EventData { get; }



        private System.Collections.Generic.IDictionary<string, object> _additionalProperties = new System.Collections.Generic.Dictionary<string, object>();

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties; }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class TabularData
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public TabularData(System.Collections.Generic.IReadOnlyList<string>? @dataChannelID, System.Collections.Generic.IReadOnlyList<DataSet_Tabular>? @dataSet, int? @numberOfDataChannel, int? @numberOfDataSet)


        {

            this.NumberOfDataSet = @numberOfDataSet;

            this.NumberOfDataChannel = @numberOfDataChannel;

            this.DataChannelID = @dataChannelID;

            this.DataSet = @dataSet;

        }
        [System.Text.Json.Serialization.JsonPropertyName("NumberOfDataSet")]
        public int? NumberOfDataSet { get; }


        [System.Text.Json.Serialization.JsonPropertyName("NumberOfDataChannel")]
        public int? NumberOfDataChannel { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataChannelID")]
        public System.Collections.Generic.IReadOnlyList<string>? DataChannelID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataSet")]
        public System.Collections.Generic.IReadOnlyList<DataSet_Tabular>? DataSet { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class EventData
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public EventData(System.Collections.Generic.IReadOnlyList<DataSet_Event>? @dataSet, int? @numberOfDataSet)


        {

            this.NumberOfDataSet = @numberOfDataSet;

            this.DataSet = @dataSet;

        }
        [System.Text.Json.Serialization.JsonPropertyName("NumberOfDataSet")]
        public int? NumberOfDataSet { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataSet")]
        public System.Collections.Generic.IReadOnlyList<DataSet_Event>? DataSet { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataSet_Tabular
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataSet_Tabular(System.Collections.Generic.IReadOnlyList<string>? @quality, System.DateTimeOffset @timeStamp, System.Collections.Generic.IReadOnlyList<string> @value)


        {

            this.TimeStamp = @timeStamp;

            this.Value = @value;

            this.Quality = @quality;

        }
        [System.Text.Json.Serialization.JsonPropertyName("TimeStamp")]
        public System.DateTimeOffset TimeStamp { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Value")]
        public System.Collections.Generic.IReadOnlyList<string> Value { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Quality")]
        public System.Collections.Generic.IReadOnlyList<string>? Quality { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataSet_Event
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataSet_Event(string @dataChannelID, string? @quality, System.DateTimeOffset @timeStamp, string @value)


        {

            this.TimeStamp = @timeStamp;

            this.DataChannelID = @dataChannelID;

            this.Value = @value;

            this.Quality = @quality;

        }
        [System.Text.Json.Serialization.JsonPropertyName("TimeStamp")]
        public System.DateTimeOffset TimeStamp { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataChannelID")]
        public string DataChannelID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Value")]
        public string Value { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Quality")]
        public string? Quality { get; }


    }

    /// <summary>
    /// A TimeSeriesData package for ISO19848
    /// </summary>
    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.6.10.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class TimeSeriesDataPackage
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public TimeSeriesDataPackage(Package @package)


        {

            this.Package = @package;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Package")]
        public Package Package { get; }


    }
}