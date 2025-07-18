"""This module generates a Python script defining the VisVersion enum and related classes."""  # noqa: E501

import argparse
from pathlib import Path

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

    with open(output_file, "w") as f:  # noqa: PTH123
        f.write("import enum\n\n")
        f.write("class VisVersion(enum.Enum):\n")
        for version in vis_versions:
            f.write(f'    v{version.replace("-", "_")} = "{version}"\n')
        f.write("\n\nclass VisVersionExtension:\n")
        f.write("    @staticmethod\n")
        f.write(
            "    def to_version_string(version: VisVersion, builder=None) -> str:\n"
        )
        f.write("        version_map = {\n")
        for version in vis_versions:
            f.write(
                f'            VisVersion.v{version.replace("-", "_")}: "{version}",\n'
            )
        f.write("        }\n")
        f.write("        v = version_map.get(version, None)\n")
        f.write("        if v is None:\n")
        f.write(
            "            raise ValueError(f'Invalid VisVersion enum value: {version}')\n"  # noqa : E501
        )
        f.write("        if builder is not None:\n")
        f.write("            builder.append(v)\n")
        f.write("        return v\n")
        f.write("\n    @staticmethod\n")
        f.write("    def to_string(version, builder=None):\n")
        f.write(
            "        return VisVersionExtension.to_version_string(version, builder)\n"
        )
        f.write("\n    @staticmethod\n")
        f.write("    def is_valid(version):\n")
        f.write("        return isinstance(version, VisVersion)\n")
        f.write("\nclass VisVersions:\n")
        f.write("    @staticmethod\n")
        f.write("    def all_versions():\n")
        f.write(
            "        return [version for version in VisVersion if VisVersions.try_parse(version.value)]\n"  # noqa : E501
        )
        f.write("    @staticmethod\n")
        f.write("    def try_parse(version_str) -> tuple[bool, VisVersion | None]:\n")
        for version in vis_versions:
            f.write(f'        if version_str == "{version}":\n')
            f.write(
                f"            return True, VisVersion.v{version.replace('-', '_')}\n"
            )
        f.write("        return False, None\n")  # Return tuple instead of raising

        # Add parse method that throws
        f.write("\n    @staticmethod\n")
        f.write("    def parse(version_str: str) -> VisVersion:\n")
        f.write("        success, version = VisVersions.try_parse(version_str)\n")
        f.write("        if not success:\n")
        f.write(
            '            raise ValueError(f"Invalid VisVersion string: {version_str}")\n'  # noqa: E501
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
    # go one level up on the path below
    resources_dir = (
        root_dir / "resources"
        if not args.resources_dir
        else Path(args.resources_dir).resolve()
    )
    output_file = root_dir / "vis_version.py"

    generate_vis_version_script(str(resources_dir), str(output_file))
