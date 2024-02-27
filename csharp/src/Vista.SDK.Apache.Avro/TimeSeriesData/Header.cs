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
	
	
	public partial class Header : ISpecificRecord
	{
		public static Schema _SCHEMA = global::Avro.Schema.Parse(@"{""type"":""record"",""name"":""Header"",""namespace"":""Vista.SDK.Transport.Avro.TimeSeriesData"",""fields"":[{""name"":""ShipID"",""type"":""string""},{""name"":""TimeSpan"",""default"":null,""type"":[""null"",{""type"":""record"",""name"":""TimeSpan"",""namespace"":""Vista.SDK.Transport.Avro.TimeSeriesData"",""fields"":[{""name"":""Start"",""type"":{""type"":""long"",""logicalType"":""timestamp-micros""}},{""name"":""End"",""type"":{""type"":""long"",""logicalType"":""timestamp-micros""}}]}]},{""name"":""DateCreated"",""default"":null,""type"":[""null"",{""type"":""long"",""logicalType"":""timestamp-micros""}]},{""name"":""DateModified"",""default"":null,""type"":[""null"",{""type"":""long"",""logicalType"":""timestamp-micros""}]},{""name"":""Author"",""default"":null,""type"":[""null"",""string""]},{""name"":""SystemConfiguration"",""default"":null,""type"":[""null"",{""type"":""array"",""items"":{""type"":""record"",""name"":""SystemConfiguration"",""namespace"":""Vista.SDK.Transport.Avro.TimeSeriesData"",""fields"":[{""name"":""ID"",""type"":""string""},{""name"":""TimeStamp"",""type"":{""type"":""long"",""logicalType"":""timestamp-micros""}}]}}]}]}");
		private string _ShipID;
		private global::Vista.SDK.Transport.Avro.TimeSeriesData.TimeSpan _TimeSpan;
		private System.Nullable<System.DateTime> _DateCreated;
		private System.Nullable<System.DateTime> _DateModified;
		private string _Author;
		private IList<global::Vista.SDK.Transport.Avro.TimeSeriesData.SystemConfiguration> _SystemConfiguration;
		public virtual Schema Schema
		{
			get
			{
				return Header._SCHEMA;
			}
		}
		public string ShipID
		{
			get
			{
				return this._ShipID;
			}
			set
			{
				this._ShipID = value;
			}
		}
		public global::Vista.SDK.Transport.Avro.TimeSeriesData.TimeSpan TimeSpan
		{
			get
			{
				return this._TimeSpan;
			}
			set
			{
				this._TimeSpan = value;
			}
		}
		public System.Nullable<System.DateTime> DateCreated
		{
			get
			{
				return this._DateCreated;
			}
			set
			{
				this._DateCreated = value;
			}
		}
		public System.Nullable<System.DateTime> DateModified
		{
			get
			{
				return this._DateModified;
			}
			set
			{
				this._DateModified = value;
			}
		}
		public string Author
		{
			get
			{
				return this._Author;
			}
			set
			{
				this._Author = value;
			}
		}
		public IList<global::Vista.SDK.Transport.Avro.TimeSeriesData.SystemConfiguration> SystemConfiguration
		{
			get
			{
				return this._SystemConfiguration;
			}
			set
			{
				this._SystemConfiguration = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.ShipID;
			case 1: return this.TimeSpan;
			case 2: return this.DateCreated;
			case 3: return this.DateModified;
			case 4: return this.Author;
			case 5: return this.SystemConfiguration;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.ShipID = (System.String)fieldValue; break;
			case 1: this.TimeSpan = (global::Vista.SDK.Transport.Avro.TimeSeriesData.TimeSpan)fieldValue; break;
			case 2: this.DateCreated = (System.Nullable<System.DateTime>)fieldValue; break;
			case 3: this.DateModified = (System.Nullable<System.DateTime>)fieldValue; break;
			case 4: this.Author = (System.String)fieldValue; break;
			case 5: this.SystemConfiguration = (IList<global::Vista.SDK.Transport.Avro.TimeSeriesData.SystemConfiguration>)fieldValue; break;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}