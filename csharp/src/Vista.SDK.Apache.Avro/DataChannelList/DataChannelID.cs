// ------------------------------------------------------------------------------
// <auto-generated>
//    Generated by avrogen, version 1.11.0.0
//    Changes to this file may cause incorrect behavior and will be lost if code
//    is regenerated
// </auto-generated>
// ------------------------------------------------------------------------------
namespace Vista.SDK.Transport.Avro.DataChannel
{
	using System;
	using System.Collections.Generic;
	using System.Text;
	using Avro;
	
	public partial class DataChannelID : ISpecificRecord
	{
		public static Schema _SCHEMA = global::Avro.Schema.Parse(@"{""type"":""record"",""name"":""DataChannelID"",""namespace"":""Vista.SDK.Transport.Avro.DataChannel"",""fields"":[{""name"":""LocalID"",""type"":""string""},{""name"":""ShortID"",""default"":null,""type"":[""null"",""string""]},{""name"":""NameObject"",""default"":null,""type"":[""null"",{""type"":""record"",""name"":""NameObject"",""namespace"":""Vista.SDK.Transport.Avro.DataChannel"",""fields"":[{""name"":""NamingRule"",""type"":""string""}]}]}]}");
		private string _LocalID;
		private string _ShortID;
		private global::Vista.SDK.Transport.Avro.DataChannel.NameObject _NameObject;
		public virtual Schema Schema
		{
			get
			{
				return DataChannelID._SCHEMA;
			}
		}
		public string LocalID
		{
			get
			{
				return this._LocalID;
			}
			set
			{
				this._LocalID = value;
			}
		}
		public string ShortID
		{
			get
			{
				return this._ShortID;
			}
			set
			{
				this._ShortID = value;
			}
		}
		public global::Vista.SDK.Transport.Avro.DataChannel.NameObject NameObject
		{
			get
			{
				return this._NameObject;
			}
			set
			{
				this._NameObject = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.LocalID;
			case 1: return this.ShortID;
			case 2: return this.NameObject;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.LocalID = (System.String)fieldValue; break;
			case 1: this.ShortID = (System.String)fieldValue; break;
			case 2: this.NameObject = (global::Vista.SDK.Transport.Avro.DataChannel.NameObject)fieldValue; break;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
