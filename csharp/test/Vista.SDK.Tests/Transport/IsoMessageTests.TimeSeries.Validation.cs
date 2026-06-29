using FluentAssertions;
using Vista.SDK.Transport;
using Vista.SDK.Transport.Json;
using Vista.SDK.Transport.TimeSeries;
using DataChannel = Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Tests.Transport;

public partial class IsoMessageTests
{
    private const string RemoteSurveyDataChannelListJson = """
        {
            "Package": {
                "Header": {
                    "ShipID": "IMO9876543",
                    "DataChannelListID": {
                        "ID": "DCL-RS-1782743923921",
                        "Version": "1",
                        "TimeStamp": "2026-06-17T08:00:00+00:00"
                    },
                    "VersionInformation": {
                        "NamingRule": "dnv",
                        "NamingSchemeVersion": "v2",
                        "ReferenceURL": "https://docs.vista.dnv.com"
                    },
                    "Author": "DNV Remote Survey",
                    "DateCreated": "2026-06-17T08:00:00+00:00",
                    "CustomHeaders": {
                        "SurveyReference": "RS-2026-04711",
                        "SurveyType": "Remote survey - drone",
                        "DroneId": "ELIOS3-SN-88231",
                        "DroneModel": "Flyability Elios 3",
                        "DroneOperator": "ACME Robotics / J. Doe",
                        "VisRelease": "3-11a"
                    }
                },
                "DataChannelList": {
                    "DataChannel": [
                        {
                            "DataChannelID": {
                                "LocalID": "/dnv-v2/vis-3-10a/107.1/meta/detail-drone.visual.corrosion",
                                "ShortID": "hull/drone/visual/inspection",
                                "NameObject": {
                                    "NamingRule": "/dnv-v2"
                                }
                            },
                            "Property": {
                                "DataChannelType": {
                                    "Type": "Inst"
                                },
                                "Format": {
                                    "Type": "String",
                                    "Restriction": {
                                        "Enumeration": [
                                            "none",
                                            "light",
                                            "moderate",
                                            "heavy",
                                            "severe"
                                        ]
                                    }
                                },
                                "Unit": {
                                    "UnitSymbol": "1",
                                    "QuantityName": "corrosion.grade"
                                },
                                "Name": "Hull monitoring | drone visual inspection - corrosion grade",
                                "Remarks": "Qualitative finding carried as an enumerated value (VIS codebooks do not standardize survey condition vocabulary)."
                            }
                        },
                        {
                            "DataChannelID": {
                                "LocalID": "/dnv-v2/vis-3-10a/102.4/meta/detail-coating.breakdown",
                                "ShortID": "coating/condition/breakdown.pct",
                                "NameObject": {
                                    "NamingRule": "/dnv-v2"
                                }
                            },
                            "Property": {
                                "DataChannelType": {
                                    "Type": "Inst"
                                },
                                "Format": {
                                    "Type": "Decimal",
                                    "Restriction": {
                                        "MaxInclusive": 100,
                                        "MinInclusive": 0
                                    }
                                },
                                "Range": {
                                    "High": 100,
                                    "Low": 0
                                },
                                "Unit": {
                                    "UnitSymbol": "%",
                                    "QuantityName": "coating.breakdown"
                                },
                                "Name": "Corrosion prevention monitoring | coating breakdown"
                            }
                        },
                        {
                            "DataChannelID": {
                                "LocalID": "/dnv-v2/vis-3-10a/111/meta/qty-thickness/pos-port",
                                "ShortID": "hull/structure/port/thickness",
                                "NameObject": {
                                    "NamingRule": "/dnv-v2"
                                }
                            },
                            "Property": {
                                "DataChannelType": {
                                    "Type": "Inst"
                                },
                                "Format": {
                                    "Type": "Decimal"
                                },
                                "Range": {
                                    "High": 1000000,
                                    "Low": 0
                                },
                                "Unit": {
                                    "UnitSymbol": "mm",
                                    "QuantityName": "thickness"
                                },
                                "Name": "Structural strength ship hull | port plate thickness (UT)"
                            }
                        },
                        {
                            "DataChannelID": {
                                "LocalID": "/dnv-v2/vis-3-10a/1021.21/meta/qty-relative.humidity",
                                "ShortID": "cargo.tank/atmosphere/humidity",
                                "NameObject": {
                                    "NamingRule": "/dnv-v2"
                                }
                            },
                            "Property": {
                                "DataChannelType": {
                                    "Type": "Inst"
                                },
                                "Format": {
                                    "Type": "Decimal"
                                },
                                "Range": {
                                    "High": 1000000,
                                    "Low": 0
                                },
                                "Unit": {
                                    "UnitSymbol": "%",
                                    "QuantityName": "relative.humidity"
                                },
                                "Name": "Cargo containment independent tank | atmosphere relative humidity"
                            }
                        },
                        {
                            "DataChannelID": {
                                "LocalID": "/dnv-v2/vis-3-10a/085/meta/detail-video.report",
                                "ShortID": "survey/report/video",
                                "NameObject": {
                                    "NamingRule": "/dnv-v2"
                                }
                            },
                            "Property": {
                                "DataChannelType": {
                                    "Type": "Inst"
                                },
                                "Format": {
                                    "Type": "String"
                                },
                                "Unit": {
                                    "UnitSymbol": "1",
                                    "QuantityName": "attachment.reference"
                                },
                                "Name": "Inspection reporting | recorded video evidence",
                                "Remarks": "Value is the survey reference. The recorded video is uploaded as a binary attachment to this package; the attachment's ParentPackageId links it back to the survey."
                            }
                        }
                    ]
                }
            }
        }
        """;

    private const string RemoteSurveyTimeSeriesDataJson = """
        {
            "Package": {
                "Header": {
                    "ShipID": "IMO9876543",
                    "TimeSpan": {
                        "Start": "2026-06-17T09:00:00+00:00",
                        "End": "2026-06-17T09:20:00+00:00"
                    },
                    "DateCreated": "2026-06-17T09:25:00+00:00",
                    "DateModified": "2026-06-17T09:25:00+00:00",
                    "Author": "DNV Remote Survey - drone capture",
                    "CustomHeaders": {
                        "SurveyReference": "RS-2026-04711",
                        "VideoFilename": "RS-2026-04711-hold3.mp4",
                        "Note": "Each TimeStamp corresponds to a moment in the recorded video."
                    }
                },
                "TimeSeriesData": [
                    {
                        "TabularData": [
                            {
                                "NumberOfDataSet": 3,
                                "NumberOfDataChannel": 2,
                                "DataChannelID": [
                                    "hull/drone/visual/inspection",
                                    "coating/condition/breakdown.pct"
                                ],
                                "DataSet": [
                                    {
                                        "TimeStamp": "2026-06-17T09:02:30+00:00",
                                        "Value": [
                                            "light",
                                            "5.0"
                                        ],
                                        "Quality": [
                                            "A",
                                            "A"
                                        ]
                                    },
                                    {
                                        "TimeStamp": "2026-06-17T09:08:10+00:00",
                                        "Value": [
                                            "moderate",
                                            "18.0"
                                        ],
                                        "Quality": [
                                            "A",
                                            "A"
                                        ]
                                    },
                                    {
                                        "TimeStamp": "2026-06-17T09:14:45+00:00",
                                        "Value": [
                                            "heavy",
                                            "42.0"
                                        ],
                                        "Quality": [
                                            "A",
                                            "A"
                                        ]
                                    }
                                ]
                            },
                            {
                                "NumberOfDataSet": 2,
                                "NumberOfDataChannel": 1,
                                "DataChannelID": [
                                    "hull/structure/port/thickness"
                                ],
                                "DataSet": [
                                    {
                                        "TimeStamp": "2026-06-17T09:08:10+00:00",
                                        "Value": [
                                            "14.6"
                                        ],
                                        "Quality": [
                                            "A"
                                        ]
                                    },
                                    {
                                        "TimeStamp": "2026-06-17T09:14:45+00:00",
                                        "Value": [
                                            "12.1"
                                        ],
                                        "Quality": [
                                            "A"
                                        ]
                                    }
                                ]
                            },
                            {
                                "NumberOfDataSet": 1,
                                "NumberOfDataChannel": 1,
                                "DataChannelID": [
                                    "cargo.tank/atmosphere/humidity"
                                ],
                                "DataSet": [
                                    {
                                        "TimeStamp": "2026-06-17T09:00:30+00:00",
                                        "Value": [
                                            "63.0"
                                        ],
                                        "Quality": [
                                            "A"
                                        ]
                                    }
                                ]
                            }
                        ]
                    },
                    {
                        "EventData": {
                            "NumberOfDataSet": 1,
                            "DataSet": [
                                {
                                    "TimeStamp": "2026-06-17T09:20:00+00:00",
                                    "DataChannelID": "survey/report/video",
                                    "Value": "RS-2026-04711",
                                    "Quality": "A"
                                }
                            ]
                        }
                    }
                ]
            }
        }
        """;

    [Fact]
    public void Test_TimeSeriesData_Validation_RegistersAllDataChannels()
    {
        var dcDto = Serializer.DeserializeDataChannelList(RemoteSurveyDataChannelListJson);
        dcDto.Should().NotBeNull();
        var dcPackage = SDK.Transport.Json.DataChannel.Extensions.ToDomainModel(dcDto!);

        var tsDto = Serializer.DeserializeTimeSeriesData(RemoteSurveyTimeSeriesDataJson);
        tsDto.Should().NotBeNull();
        var tsPackage = SDK.Transport.Json.TimeSeriesData.Extensions.ToDomainModel(tsDto!);

        var registeredDataChannels = new Dictionary<string, DataChannel.DataChannel>();

        ValidateData register = (timestamp, dataChannel, value, quality) =>
        {
            var key = dataChannel.DataChannelId.ShortId ?? dataChannel.DataChannelId.LocalId.ToString();
            registeredDataChannels[key] = dataChannel;
            return new ValidateResult.Ok();
        };

        foreach (var tsData in tsPackage.Package.TimeSeriesData)
        {
            var result = tsData.Validate(dcPackage, onTabularData: register, onEventData: register);

            result.Should().BeOfType<ValidateResult.Ok>();
        }

        var expectedDataChannelIds = new[]
        {
            "hull/drone/visual/inspection",
            "coating/condition/breakdown.pct",
            "hull/structure/port/thickness",
            "cargo.tank/atmosphere/humidity",
            "survey/report/video"
        };

        registeredDataChannels.Should().HaveCount(5);
        registeredDataChannels.Keys.Should().BeEquivalentTo(expectedDataChannelIds);
    }
}
