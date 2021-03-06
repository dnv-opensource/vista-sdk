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
	
	
	public partial class Property : ISpecificRecord
	{
		public static Schema _SCHEMA = global::Avro.Schema.Parse("{\"type\":\"record\",\"name\":\"Property\",\"namespace\":\"Vista.SDK.Transport.Avro.DataChanne" +
				"l\",\"fields\":[{\"name\":\"DataChannelType\",\"type\":{\"type\":\"record\",\"name\":\"DataChann" +
				"elType\",\"namespace\":\"Vista.SDK.Transport.Avro.DataChannel\",\"fields\":[{\"name\":\"Type" +
				"\",\"type\":\"string\"},{\"name\":\"UpdateCycle\",\"default\":null,\"type\":[\"null\",\"string\"]" +
				"},{\"name\":\"CalculationPeriod\",\"default\":null,\"type\":[\"null\",\"string\"]}]}},{\"name" +
				"\":\"Format\",\"type\":{\"type\":\"record\",\"name\":\"Format\",\"namespace\":\"Vista.SDK.Transpor" +
				"t.Avro.DataChannel\",\"fields\":[{\"name\":\"Type\",\"type\":\"string\"},{\"name\":\"Restricti" +
				"on\",\"default\":null,\"type\":[\"null\",{\"type\":\"record\",\"name\":\"Restriction\",\"namespa" +
				"ce\":\"Vista.SDK.Transport.Avro.DataChannel\",\"fields\":[{\"name\":\"Enumeration\",\"defaul" +
				"t\":null,\"type\":[\"null\",{\"type\":\"array\",\"items\":\"string\"}]},{\"name\":\"FractionDigi" +
				"ts\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"Length\",\"default\":null,\"ty" +
				"pe\":[\"null\",\"string\"]},{\"name\":\"MaxExclusive\",\"default\":null,\"type\":[\"null\",\"str" +
				"ing\"]},{\"name\":\"MaxInclusive\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"" +
				"MaxLength\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"MinExclusive\",\"defa" +
				"ult\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"MinInclusive\",\"default\":null,\"type\"" +
				":[\"null\",\"string\"]},{\"name\":\"MinLength\",\"default\":null,\"type\":[\"null\",\"string\"]}" +
				",{\"name\":\"Pattern\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"TotalDigits" +
				"\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"WhiteSpace\",\"default\":null,\"" +
				"type\":[\"null\",{\"type\":\"enum\",\"name\":\"WhiteSpace\",\"namespace\":\"Vista.SDK.Transport." +
				"Avro.DataChannel\",\"symbols\":[\"Preserve\",\"Replace\",\"Collapse\"]}]}]}]}]}},{\"name\":" +
				"\"Range\",\"default\":null,\"type\":[\"null\",{\"type\":\"record\",\"name\":\"Range\",\"namespace" +
				"\":\"Vista.SDK.Transport.Avro.DataChannel\",\"fields\":[{\"name\":\"High\",\"type\":\"string\"}" +
				",{\"name\":\"Low\",\"type\":\"string\"}]}]},{\"name\":\"Unit\",\"default\":null,\"type\":[\"null\"" +
				",{\"type\":\"record\",\"name\":\"Unit\",\"namespace\":\"Vista.SDK.Transport.Avro.DataChannel\"" +
				",\"fields\":[{\"name\":\"UnitSymbol\",\"type\":\"string\"},{\"name\":\"QuantityName\",\"default" +
				"\":null,\"type\":[\"null\",\"string\"]}]}]},{\"name\":\"QualityCoding\",\"default\":null,\"typ" +
				"e\":[\"null\",\"string\"]},{\"name\":\"AlertPriority\",\"default\":null,\"type\":[\"null\",\"str" +
				"ing\"]},{\"name\":\"Name\",\"default\":null,\"type\":[\"null\",\"string\"]},{\"name\":\"Remarks\"" +
				",\"default\":null,\"type\":[\"null\",\"string\"]}]}");
		private global::Vista.SDK.Transport.Avro.DataChannel.DataChannelType _DataChannelType;
		private global::Vista.SDK.Transport.Avro.DataChannel.Format _Format;
		private global::Vista.SDK.Transport.Avro.DataChannel.Range _Range;
		private global::Vista.SDK.Transport.Avro.DataChannel.Unit _Unit;
		private string _QualityCoding;
		private string _AlertPriority;
		private string _Name;
		private string _Remarks;
		public virtual Schema Schema
		{
			get
			{
				return Property._SCHEMA;
			}
		}
		public global::Vista.SDK.Transport.Avro.DataChannel.DataChannelType DataChannelType
		{
			get
			{
				return this._DataChannelType;
			}
			set
			{
				this._DataChannelType = value;
			}
		}
		public global::Vista.SDK.Transport.Avro.DataChannel.Format Format
		{
			get
			{
				return this._Format;
			}
			set
			{
				this._Format = value;
			}
		}
		public global::Vista.SDK.Transport.Avro.DataChannel.Range Range
		{
			get
			{
				return this._Range;
			}
			set
			{
				this._Range = value;
			}
		}
		public global::Vista.SDK.Transport.Avro.DataChannel.Unit Unit
		{
			get
			{
				return this._Unit;
			}
			set
			{
				this._Unit = value;
			}
		}
		public string QualityCoding
		{
			get
			{
				return this._QualityCoding;
			}
			set
			{
				this._QualityCoding = value;
			}
		}
		public string AlertPriority
		{
			get
			{
				return this._AlertPriority;
			}
			set
			{
				this._AlertPriority = value;
			}
		}
		public string Name
		{
			get
			{
				return this._Name;
			}
			set
			{
				this._Name = value;
			}
		}
		public string Remarks
		{
			get
			{
				return this._Remarks;
			}
			set
			{
				this._Remarks = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.DataChannelType;
			case 1: return this.Format;
			case 2: return this.Range;
			case 3: return this.Unit;
			case 4: return this.QualityCoding;
			case 5: return this.AlertPriority;
			case 6: return this.Name;
			case 7: return this.Remarks;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.DataChannelType = (global::Vista.SDK.Transport.Avro.DataChannel.DataChannelType)fieldValue; break;
			case 1: this.Format = (global::Vista.SDK.Transport.Avro.DataChannel.Format)fieldValue; break;
			case 2: this.Range = (global::Vista.SDK.Transport.Avro.DataChannel.Range)fieldValue; break;
			case 3: this.Unit = (global::Vista.SDK.Transport.Avro.DataChannel.Unit)fieldValue; break;
			case 4: this.QualityCoding = (System.String)fieldValue; break;
			case 5: this.AlertPriority = (System.String)fieldValue; break;
			case 6: this.Name = (System.String)fieldValue; break;
			case 7: this.Remarks = (System.String)fieldValue; break;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
