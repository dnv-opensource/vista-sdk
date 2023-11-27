using System.Diagnostics;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;

namespace Vista.SDK.SourceGenerator
{
    [Generator]
    public sealed partial class VISGenerator : ISourceGenerator
    {
        public void Execute(GeneratorExecutionContext context)
        {
            //System.Diagnostics.Debugger.Launch();
            var compilation = context.Compilation;

            if (compilation.AssemblyName != "Vista.SDK")
                return;

            var cancellationToken = context.CancellationToken;

            var visVersions = EmbeddedResource.GetVisVersions();

            if (visVersions is null)
            {
                throw new Exception("Couldn't find any VIS versions for gmod");
            }

            var sourceBuilder = new StringBuilder(
                @"
#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Vista.SDK
{
    public enum VisVersion
    {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
        [global::System.Runtime.Serialization.EnumMember(Value = ""{version}"")]
        v{version.Replace('-', '_')},"
                );
            }

            sourceBuilder.Append(
                @"
    }

    static partial class VisVersionExtensions
    {
        public static string ToVersionString(this VisVersion version) =>
            version switch
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                VisVersion.v{version.Replace('-', '_')} => ""{version}"","
                );
            }

            sourceBuilder.Append(
                @"
                _ => throw new System.InvalidOperationException(""Invalid VisVersion enum value""),
            };

        public static string ToString(this VisVersion version) =>
            version switch
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                VisVersion.v{version.Replace('-', '_')} => ""{version}"","
                );
            }

            sourceBuilder.Append(
                @"
                _ => throw new System.InvalidOperationException(""Invalid VisVersion enum value""),
            };

        public static bool IsValid(this VisVersion version) =>
            version switch
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                VisVersion.v{version.Replace('-', '_')} => true,"
                );
            }

            sourceBuilder.Append(
                @"
                _ => false,
            };


        public static void ToVersionString(this VisVersion version, global::System.Text.StringBuilder builder)
        {
            switch (version)
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                case VisVersion.v{version.Replace('-', '_')}:
                    builder.Append(""{version}"");
                    break;
                "
                );
            }

            sourceBuilder.Append(
                @"
                default:
                    throw new System.InvalidOperationException(""Invalid VisVersion enum value"");
            }
        }


        public static void ToString(this VisVersion version, global::System.Text.StringBuilder builder)
        {
            switch (version)
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                case VisVersion.v{version.Replace('-', '_')}:
                    builder.Append(""{version}"");
                    break;
                "
                );
            }

            sourceBuilder.Append(
                @"
                default:
                    throw new System.InvalidOperationException(""Invalid VisVersion enum value"");
            }
        }
    }"
            );

            sourceBuilder.Append(
                @"
    public static class VisVersions
    {
        public static readonly global::System.Collections.Generic.IEnumerable<VisVersion> All = new []
        {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                VisVersion.v{version.Replace('-', '_')},
                "
                );
            }

            sourceBuilder.Append(
                @"
        };

        public static VisVersion Parse(string version)
        {
            return version switch
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                ""{version}"" => VisVersion.v{version.Replace('-', '_')},"
                );
            }

            sourceBuilder.Append(
                @"
                _ => throw new System.ArgumentException(""Invalid VIS version input: "" + version)
            };
        }

        public static VisVersion Parse(global::System.ReadOnlySpan<char> version)
        {
            "
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
            if (version.SequenceEqual(""{version}"".AsSpan()))
                return VisVersion.v{version.Replace('-', '_')};
                "
                );
            }

            sourceBuilder.Append(
                @"

            throw new System.ArgumentException(""Invalid VIS version input: "" + version.ToString());
        }

        public static bool TryParse(string versionStr, out VisVersion version)
        {
            switch (versionStr)
            {"
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
                case ""{version}"":
                    version = VisVersion.v{version.Replace('-', '_')};
                    return true;
                "
                );
            }

            sourceBuilder.Append(
                @"
                default:
                    version = default;
                    return false;
            }
        }

        public static bool TryParse(global::System.ReadOnlySpan<char> versionStr, out VisVersion version)
        {
            "
            );

            foreach (var version in visVersions)
            {
                sourceBuilder.Append(
                    @$"
            if (versionStr.SequenceEqual(""{version}"".AsSpan()))
            {{
                version = VisVersion.v{version.Replace('-', '_')};
                return true;
            }}
                "
                );
            }

            sourceBuilder.Append(
                @"
            version = default;
            return false;
        }
    }
"
            );

            sourceBuilder.Append(
                @"
}"
            );

            //Debugger.Launch();

            // inject the created source into the users compilation
            context.AddSource("VisVersionExtensions.g.cs", SourceText.From(sourceBuilder.ToString(), Encoding.UTF8)); // VisVersionExtensions.g

            Debug.WriteLine($"{nameof(VISGenerator)} generated source code");
        }

        public void Initialize(GeneratorInitializationContext context) { }
    }
}
