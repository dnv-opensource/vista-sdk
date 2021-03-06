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
	
	
	public partial class Restriction : ISpecificRecord
	{
		public static Schema _SCHEMA = global::Avro.Schema.Parse(@"{""type"":""record"",""name"":""Restriction"",""namespace"":""Vista.SDK.Transport.Avro.DataChannel"",""fields"":[{""name"":""Enumeration"",""default"":null,""type"":[""null"",{""type"":""array"",""items"":""string""}]},{""name"":""FractionDigits"",""default"":null,""type"":[""null"",""string""]},{""name"":""Length"",""default"":null,""type"":[""null"",""string""]},{""name"":""MaxExclusive"",""default"":null,""type"":[""null"",""string""]},{""name"":""MaxInclusive"",""default"":null,""type"":[""null"",""string""]},{""name"":""MaxLength"",""default"":null,""type"":[""null"",""string""]},{""name"":""MinExclusive"",""default"":null,""type"":[""null"",""string""]},{""name"":""MinInclusive"",""default"":null,""type"":[""null"",""string""]},{""name"":""MinLength"",""default"":null,""type"":[""null"",""string""]},{""name"":""Pattern"",""default"":null,""type"":[""null"",""string""]},{""name"":""TotalDigits"",""default"":null,""type"":[""null"",""string""]},{""name"":""WhiteSpace"",""default"":null,""type"":[""null"",{""type"":""enum"",""name"":""WhiteSpace"",""namespace"":""Vista.SDK.Transport.Avro.DataChannel"",""symbols"":[""Preserve"",""Replace"",""Collapse""]}]}]}");
		private IList<System.String> _Enumeration;
		private string _FractionDigits;
		private string _Length;
		private string _MaxExclusive;
		private string _MaxInclusive;
		private string _MaxLength;
		private string _MinExclusive;
		private string _MinInclusive;
		private string _MinLength;
		private string _Pattern;
		private string _TotalDigits;
		private System.Nullable<global::Vista.SDK.Transport.Avro.DataChannel.WhiteSpace> _WhiteSpace;
		public virtual Schema Schema
		{
			get
			{
				return Restriction._SCHEMA;
			}
		}
		public IList<System.String> Enumeration
		{
			get
			{
				return this._Enumeration;
			}
			set
			{
				this._Enumeration = value;
			}
		}
		public string FractionDigits
		{
			get
			{
				return this._FractionDigits;
			}
			set
			{
				this._FractionDigits = value;
			}
		}
		public string Length
		{
			get
			{
				return this._Length;
			}
			set
			{
				this._Length = value;
			}
		}
		public string MaxExclusive
		{
			get
			{
				return this._MaxExclusive;
			}
			set
			{
				this._MaxExclusive = value;
			}
		}
		public string MaxInclusive
		{
			get
			{
				return this._MaxInclusive;
			}
			set
			{
				this._MaxInclusive = value;
			}
		}
		public string MaxLength
		{
			get
			{
				return this._MaxLength;
			}
			set
			{
				this._MaxLength = value;
			}
		}
		public string MinExclusive
		{
			get
			{
				return this._MinExclusive;
			}
			set
			{
				this._MinExclusive = value;
			}
		}
		public string MinInclusive
		{
			get
			{
				return this._MinInclusive;
			}
			set
			{
				this._MinInclusive = value;
			}
		}
		public string MinLength
		{
			get
			{
				return this._MinLength;
			}
			set
			{
				this._MinLength = value;
			}
		}
		public string Pattern
		{
			get
			{
				return this._Pattern;
			}
			set
			{
				this._Pattern = value;
			}
		}
		public string TotalDigits
		{
			get
			{
				return this._TotalDigits;
			}
			set
			{
				this._TotalDigits = value;
			}
		}
		public System.Nullable<global::Vista.SDK.Transport.Avro.DataChannel.WhiteSpace> WhiteSpace
		{
			get
			{
				return this._WhiteSpace;
			}
			set
			{
				this._WhiteSpace = value;
			}
		}
		public virtual object Get(int fieldPos)
		{
			switch (fieldPos)
			{
			case 0: return this.Enumeration;
			case 1: return this.FractionDigits;
			case 2: return this.Length;
			case 3: return this.MaxExclusive;
			case 4: return this.MaxInclusive;
			case 5: return this.MaxLength;
			case 6: return this.MinExclusive;
			case 7: return this.MinInclusive;
			case 8: return this.MinLength;
			case 9: return this.Pattern;
			case 10: return this.TotalDigits;
			case 11: return this.WhiteSpace;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Get()");
			};
		}
		public virtual void Put(int fieldPos, object fieldValue)
		{
			switch (fieldPos)
			{
			case 0: this.Enumeration = (IList<System.String>)fieldValue; break;
			case 1: this.FractionDigits = (System.String)fieldValue; break;
			case 2: this.Length = (System.String)fieldValue; break;
			case 3: this.MaxExclusive = (System.String)fieldValue; break;
			case 4: this.MaxInclusive = (System.String)fieldValue; break;
			case 5: this.MaxLength = (System.String)fieldValue; break;
			case 6: this.MinExclusive = (System.String)fieldValue; break;
			case 7: this.MinInclusive = (System.String)fieldValue; break;
			case 8: this.MinLength = (System.String)fieldValue; break;
			case 9: this.Pattern = (System.String)fieldValue; break;
			case 10: this.TotalDigits = (System.String)fieldValue; break;
			case 11: this.WhiteSpace = fieldValue == null ? (System.Nullable<global::Vista.SDK.Transport.Avro.DataChannel.WhiteSpace>)null : (global::Vista.SDK.Transport.Avro.DataChannel.WhiteSpace)fieldValue; break;
			default: throw new AvroRuntimeException("Bad index " + fieldPos + " in Put()");
			};
		}
	}
}
