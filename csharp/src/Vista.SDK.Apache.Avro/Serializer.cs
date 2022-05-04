using Avro.IO;
using Vista.SDK.Transport.Avro.DataChannel;
using Vista.SDK.Transport.Avro.TimeSeriesData;

namespace Vista.SDK.Transport.Apache.Avro;

public static class Serializer
{
    public static void Serialize(this TimeSeriesDataPackage dto, Stream writer)
    {
        var avroWriter = new SpecificWriter<TimeSeriesDataPackage>(TimeSeriesDataPackage._SCHEMA);
        var encoder = new BinaryEncoder(writer);
        avroWriter.Write(dto, encoder);
    }

    public static void Serialize(this DataChannelListPackage dto, Stream writer)
    {
        var avroWriter = new SpecificWriter<DataChannelListPackage>(DataChannelListPackage._SCHEMA);
        var encoder = new BinaryEncoder(writer);
        avroWriter.Write(dto, encoder);
    }

    public static TimeSeriesDataPackage DeserializeTimeSeriesData(Stream reader)
    {
        var avroReader = new SpecificReader<TimeSeriesDataPackage>(
            TimeSeriesDataPackage._SCHEMA,
            TimeSeriesDataPackage._SCHEMA
        );
        var decoder = new BinaryDecoder(reader);
        var package = avroReader.Read(new TimeSeriesDataPackage(), decoder);
        return package;
    }

    public static DataChannelListPackage DeserializeDataChannelList(Stream reader)
    {
        var avroReader = new SpecificReader<DataChannelListPackage>(
            DataChannelListPackage._SCHEMA,
            DataChannelListPackage._SCHEMA
        );
        var decoder = new BinaryDecoder(reader);
        var package = avroReader.Read(new DataChannelListPackage(), decoder);
        return package;
    }
}
