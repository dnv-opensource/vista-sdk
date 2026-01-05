"""This module generates a Python script defining the VisVersion enum and related classes."""

import argparse
from pathlib import Path

# You'll need to implement or import this properly
from vista_sdk.source_generator.embedded_resources import EmbeddedResource


def generate_vis_version_script(directory: str, output_file: str) -> None:
    """Generates a Python script defining the VisVersion enum and related classes."""
    output_dir = Path(output_file).parent
    if not output_dir.exists():
        output_dir.mkdir(parents=True, exist_ok=True)

    vis_versions = EmbeddedResource.get_vis_versions(directory)

    if not vis_versions:
        raise ValueError("No VIS versions found in resources")

    # Validate version format
    for version in vis_versions:
        if not version or not isinstance(version, str):
            raise ValueError(f"Invalid version format: {version}")
        if not version.replace("-", "").replace(".", "").isalnum():
            raise ValueError(f"Version contains invalid characters: {version}")

    with Path(output_file).open("w", encoding="utf-8") as f:
        # Write auto-generated header
        f.write(
            "# =============================================================================\n"
        )
        f.write("# AUTO-GENERATED FILE - DO NOT EDIT MANUALLY\n")
        f.write(
            "# =============================================================================\n"
        )
        f.write("#\n")
        f.write("# This file is generated from the resource files in resources/\n")
        f.write("#\n")
        f.write("# To regenerate after adding new VIS versions:\n")
        f.write("#\n")
        f.write("#   cd python\n")
        f.write(
            "#   uv run python src/vista_sdk/source_generator/vis_versions_generator.py\n"
        )
        f.write("#\n")
        f.write(
            "# =============================================================================\n"
        )
        f.write("\n")
        f.write('"""Module providing VIS version enumeration and utilities."""\n\n')
        f.write("from __future__ import annotations\n\n")
        f.write("import enum\n\n\n")

        # Write VisVersion enum
        f.write("class VisVersion(enum.Enum):\n")
        f.write('    """Enumeration of VIS versions.\n\n')
        f.write(
            "    Represents the various versions of the Vessel Information Structure (VIS).\n"
        )
        f.write('    """\n\n')

        for version in vis_versions:
            enum_name = version.replace("-", "_").replace(".", "_")
            f.write(f'    v{enum_name} = "{version}"\n')

        # Write VisVersionExtension class
        f.write("\n\nclass VisVersionExtension:\n")
        f.write('    """Utility class for VIS version string manipulation."""\n\n')

        f.write("    @staticmethod\n")
        f.write(
            "    def to_version_string(version: VisVersion, builder: list[str] | None = None) -> str:\n"
        )
        f.write(
            '        """Convert a VisVersion enum to its string representation."""\n'
        )
        f.write("        version_map = {\n")
        for version in vis_versions:
            enum_name = version.replace("-", "_").replace(".", "_")
            f.write(f'            VisVersion.v{enum_name}: "{version}",\n')
        f.write("        }\n")
        f.write("        v = version_map.get(version)\n")
        f.write("        if v is None:\n")
        f.write(
            '            raise ValueError(f"Invalid VisVersion enum value: {version}")\n'
        )
        f.write("        if builder is not None:\n")
        f.write("            builder.append(v)\n")
        f.write("        return v\n")

        f.write("\n    @staticmethod\n")
        f.write(
            "    def to_string(version: VisVersion, builder: list[str] | None = None) -> str:\n"
        )
        f.write(
            '        """Convert a VisVersion enum to its string representation."""\n'
        )
        f.write(
            "        return VisVersionExtension.to_version_string(version, builder)\n"
        )

        f.write("\n    @staticmethod\n")
        f.write("    def is_valid(version: object) -> bool:\n")
        f.write('        """Check if a version is valid."""\n')
        f.write("        return isinstance(version, VisVersion)\n")

        # Write VisVersions class
        f.write("\n\nclass VisVersions:\n")
        f.write('    """Utility class for VIS version management."""\n\n')

        f.write("    @staticmethod\n")
        f.write("    def all_versions() -> list[VisVersion]:\n")
        f.write('        """Get all available VIS versions."""\n')
        f.write("        return [\n")
        f.write(
            "            version for version in VisVersion if VisVersions.try_parse(version.value)\n"
        )
        f.write("        ]\n")

        f.write("\n    @staticmethod\n")
        f.write("    def try_parse(version_str: str) -> VisVersion | None:\n")
        f.write('        """Try to parse a string into a VisVersion enum."""\n')
        f.write("        version_map = {\n")
        for version in vis_versions:
            enum_name = version.replace("-", "_").replace(".", "_")
            f.write(f'            "{version}": VisVersion.v{enum_name},\n')
        f.write("        }\n")
        f.write("        if version_str not in version_map:\n")
        f.write(
            '            raise ValueError(f"Invalid VisVersion string: {version_str}")\n'
        )
        f.write("        return version_map[version_str]\n")

        f.write("\n    @staticmethod\n")
        f.write("    def parse(version_str: str) -> VisVersion:\n")
        f.write('        """Parse a string into a VisVersion enum."""\n')
        f.write("        version = VisVersions.try_parse(version_str)\n")
        f.write("        if version is None:\n")
        f.write(
            '            raise ValueError(f"Invalid VisVersion string: {version_str}")\n'
        )
        f.write("        return version\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate VisVersion script.")
    parser.add_argument(
        "--resources_dir",
        type=str,
        required=False,
        help="The directory containing the .gz files",
    )
    args = parser.parse_args()

    root_dir = Path(__file__).parent.parent.resolve()
    resources_dir = (
        root_dir / "resources"
        if not args.resources_dir
        else Path(args.resources_dir).resolve()
    )
    output_file = root_dir / "vis_version.py"

    generate_vis_version_script(str(resources_dir), str(output_file))
