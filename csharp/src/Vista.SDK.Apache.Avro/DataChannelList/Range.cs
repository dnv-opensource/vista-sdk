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
	
	
	public partial class Range : ISpecificRecord
	{
		public static Schema _SCHEMA = global::Avro.Schema.Parse("{\"type\":\"record\",\"name\":\"Range\",\"namespace\":\"Vista.SDK.Transport.Avro.DataChannel\"," +
				"\"fields\":[{\"name\":\"High\",\"type\":\"string\"},{\"name\":\"Low\",\"type\":\"string\"}]}");
		private string _High;
		private string _Low;
		public virtual Schema Schema
		{
			get
			{
				return Range._SCHEMA;
			}
		}
		public string High
		{
			get
			{
				return this._High;
			}
			set
			{
				this._High = value;
			}
		}
		public string Low
		{
			get
			{
				return this._Low;
			}
			set
			{
				this._Low = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.High;
			case 1: return this.Low;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.High = (System.String)fieldValue; break;
			case 1: this.Low = (System.String)fieldValue; break;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}