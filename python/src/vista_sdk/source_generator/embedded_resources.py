"""This module provides functionality to handle embedded resources for the VISTA SDK.

It includes methods to retrieve resource names, decompress files,
and extract VISTA versions.
"""

import gzip
import sys
from pathlib import Path

from python.src.vista_sdk.gmod_dto import GmodDto

root_dir = (Path(__file__).parent.parent).resolve()
sys.path.append(str(root_dir))


class EmbeddedResource:
    """Class to handle embedded resources for VISTA SDK."""

    @staticmethod
    def get_resource_names(directory: str) -> list[str]:
        """Get the names of all .gz files in the specified directory."""
        from pathlib import Path

        return [
            f.name
            for f in Path(directory).iterdir()
            if f.is_file() and f.name.endswith(".gz")
        ]

    @staticmethod
    def get_decompressed_stream(filepath: str) -> str:
        """Read and decompress a .gz file, returning its content as a string."""
        with gzip.open(filepath, "rt", encoding="utf-8") as f:
            return f.read()

    @staticmethod
    def get_vis_versions(directory: str) -> list[str]:
        """Retrieve all VISTA versions from the embedded resources in the specified directory."""  # noqa: E501
        resource_names = EmbeddedResource.get_resource_names(directory)
        if not resource_names:
            raise Exception(
                f"Did not find required resources in directory '{directory}'."
            )

        vis_versions = []
        for resource_name in resource_names:
            if "gmod" in resource_name and "versioning" not in resource_name:
                from pathlib import Path

                stream_content = EmbeddedResource.get_decompressed_stream(
                    str(Path(directory) / resource_name)
                )
                gmod = GmodDto.model_validate(stream_content)
                vis_versions.append(gmod.vis_version)

        return vis_versions
