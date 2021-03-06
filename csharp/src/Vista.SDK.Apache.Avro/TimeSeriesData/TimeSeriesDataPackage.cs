// ------------------------------------------------------------------------------
// <auto-generated>
//    Generated by avrogen, version 1.11.0.0
//    Changes to this file may cause incorrect behavior and will be lost if code
//    is regenerated
// </auto-generated>
// ------------------------------------------------------------------------------
namespace Vista.SDK.Transport.Avro.TimeSeriesData
{
	using System;
	using System.Collections.Generic;
	using System.Text;
	using Avro;
	
	
	/// <summary>
	/// A TimeSeriesData package for ISO19848
	/// </summary>
	public partial class TimeSeriesDataPackage : ISpecificRecord
	{
		public static Schema _SCHEMA = global::Avro.Schema.Parse("{\"type\":\"record\",\"name\":\"TimeSeriesDataPackage\",\"doc\":\"A TimeSeriesData package f" +
				"or ISO19848\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeriesData\",\"fields\":[{\"nam" +
				"e\":\"Package\",\"type\":{\"type\":\"record\",\"name\":\"Package\",\"namespace\":\"Vista.SDK.Trans" +
				"port.Avro.TimeSeriesData\",\"fields\":[{\"name\":\"Header\",\"default\":null,\"type\":[\"nul" +
				"l\",{\"type\":\"record\",\"name\":\"Header\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeri" +
				"esData\",\"fields\":[{\"name\":\"ShipID\",\"type\":\"string\"},{\"name\":\"TimeSpan\",\"default\"" +
				":null,\"type\":[\"null\",{\"type\":\"record\",\"name\":\"TimeSpan\",\"namespace\":\"Vista.SDK.Tra" +
				"nsport.Avro.TimeSeriesData\",\"fields\":[{\"name\":\"Start\",\"type\":{\"type\":\"long\",\"log" +
				"icalType\":\"timestamp-micros\"}},{\"name\":\"End\",\"type\":{\"type\":\"long\",\"logicalType\"" +
				":\"timestamp-micros\"}}]}]},{\"name\":\"DateCreated\",\"default\":null,\"type\":[\"null\",{\"" +
				"type\":\"long\",\"logicalType\":\"timestamp-micros\"}]},{\"name\":\"DateModified\",\"default" +
				"\":null,\"type\":[\"null\",{\"type\":\"long\",\"logicalType\":\"timestamp-micros\"}]},{\"name\"" +
				":\"Author\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"SystemConfiguration\"" +
				",\"default\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":{\"type\":\"record\",\"name\":\"" +
				"SystemConfiguration\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeriesData\",\"fields" +
				"\":[{\"name\":\"ID\",\"type\":\"string\"},{\"name\":\"TimeStamp\",\"type\":{\"type\":\"long\",\"logi" +
				"calType\":\"timestamp-micros\"}}]}}]}]}]},{\"name\":\"TimeSeriesData\",\"type\":{\"type\":\"" +
				"array\",\"items\":{\"type\":\"record\",\"name\":\"TimeSeriesData\",\"namespace\":\"Vista.SDK.Tra" +
				"nsport.Avro.TimeSeriesData\",\"fields\":[{\"name\":\"DataConfiguration\",\"default\":null" +
				",\"type\":[\"null\",{\"type\":\"record\",\"name\":\"DataConfiguration\",\"namespace\":\"Vista.SDK" +
				".Transport.Avro.TimeSeriesData\",\"fields\":[{\"name\":\"ID\",\"type\":\"string\"},{\"name\":" +
				"\"TimeStamp\",\"type\":{\"type\":\"long\",\"logicalType\":\"timestamp-micros\"}}]}]},{\"name\"" +
				":\"TabularData\",\"default\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":{\"type\":\"re" +
				"cord\",\"name\":\"TabularData\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeriesData\",\"" +
				"fields\":[{\"name\":\"NumberOfDataSet\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"na" +
				"me\":\"NumberOfDataChannel\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"Data" +
				"ChannelID\",\"default\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":\"string\"}]},{\"n" +
				"ame\":\"DataSet\",\"default\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":{\"type\":\"re" +
				"cord\",\"name\":\"TabularDataSet\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeriesData" +
				"\",\"fields\":[{\"name\":\"TimeStamp\",\"type\":{\"type\":\"long\",\"logicalType\":\"timestamp-m" +
				"icros\"}},{\"name\":\"Value\",\"type\":{\"type\":\"array\",\"items\":\"string\"}},{\"name\":\"Qual" +
				"ity\",\"default\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":\"string\"}]}]}}]}]}}]}" +
				",{\"name\":\"EventData\",\"default\":null,\"type\":[\"null\",{\"type\":\"record\",\"name\":\"Even" +
				"tData\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeriesData\",\"fields\":[{\"name\":\"Nu" +
				"mberOfDataSet\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"DataSet\",\"defau" +
				"lt\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":{\"type\":\"record\",\"name\":\"EventDa" +
				"taSet\",\"namespace\":\"Vista.SDK.Transport.Avro.TimeSeriesData\",\"fields\":[{\"name\":\"Ti" +
				"meStamp\",\"type\":{\"type\":\"long\",\"logicalType\":\"timestamp-micros\"}},{\"name\":\"DataC" +
				"hannelID\",\"type\":\"string\"},{\"name\":\"Value\",\"type\":\"string\"},{\"name\":\"Quality\",\"d" +
				"efault\":null,\"type\":[\"null\",\"string\"]}]}}]}]}]}]}}}]}}]}");
		private global::Vista.SDK.Transport.Avro.TimeSeriesData.Package _Package;
		public virtual Schema Schema
		{
			get
			{
				return TimeSeriesDataPackage._SCHEMA;
			}
		}
		public global::Vista.SDK.Transport.Avro.TimeSeriesData.Package Package
		{
			get
			{
				return this._Package;
			}
			set
			{
				this._Package = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.Package;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.Package = (global::Vista.SDK.Transport.Avro.TimeSeriesData.Package)fieldValue; break;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
