from src.SourceGenerator.EmbeddedResources import EmbeddedResource


@staticmethod
def generate_vis_version_script(directory: str, output_file: str):
    vis_versions = EmbeddedResource.get_vis_versions(directory)

    with open(output_file, 'w') as f:
        f.write("import enum\n\n")
        f.write("class VisVersion(enum.Enum):\n")
        for version in vis_versions:
            f.write(f"    v{version.replace('-', '_')} = \"{version}\"\n")
        f.write("\n\nclass VisVersionExtension:\n")
        f.write("    @staticmethod\n")
        f.write("    def to_version_string(version: VisVersion, builder=None) -> str:\n")
        f.write("        version_map = {\n")
        for version in vis_versions:
            f.write(f"            VisVersion.v{version.replace('-', '_')}: \"{version}\",\n")
        f.write("        }\n")
        f.write("        v = version_map.get(version, None)\n")
        f.write("        if v is None:\n")
        f.write("            raise ValueError(f'Invalid VisVersion enum value: {version}')\n")
        f.write("        if builder is not None:\n")
        f.write("            builder.append(v)\n")
        f.write("        return v\n")
        f.write("\n    @staticmethod\n")
        f.write("    def to_string(version, builder=None):\n")
        f.write("        return VisVersionExtension.to_version_string(version, builder)\n")
        f.write("\n    @staticmethod\n")
        f.write("    def is_valid(version):\n")
        f.write("        return isinstance(version, VisVersion)\n")
        f.write("\nclass VisVersions:\n")
        f.write("    @staticmethod\n")
        f.write("    def all_versions():\n")
        f.write("        return [version for version in VisVersion if VisVersions.try_parse(version.value)]\n")
        f.write("\n    @staticmethod\n")
        f.write("    def try_parse(version_str) -> VisVersion:\n")
        for version in vis_versions:
            f.write(f"        if version_str == \"{version}\":\n")
            f.write(f"            return VisVersion.v{version.replace('-', '_')}\n")
        f.write("        return None\n")
        f.write("\n    @staticmethod\n")
        f.write("    def parse(version_str: str) -> VisVersion:\n")
        f.write("        version = VisVersions.try_parse(version_str)\n")
        f.write("        if version is None:\n")
        f.write("            raise ValueError(f\"Invalid VisVersion string: {version_str}\")\n")
        f.write("        return version\n")
