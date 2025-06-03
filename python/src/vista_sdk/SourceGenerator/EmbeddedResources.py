import gzip
import os
import sys
from typing import List

from vista_sdk.GmodDto import GmodDto

root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.append(root_dir)


class EmbeddedResource:
    @staticmethod
    def get_resource_names(directory: str) -> List[str]:
        return [f for f in os.listdir(directory) if f.endswith(".gz")]

    @staticmethod
    def get_decompressed_stream(filepath: str) -> str:
        with gzip.open(filepath, "rt", encoding="utf-8") as f:
            return f.read()

    @staticmethod
    def get_vis_versions(directory: str) -> List[str]:
        resource_names = EmbeddedResource.get_resource_names(directory)
        if not resource_names:
            raise Exception(
                f"Did not find required resources in directory '{directory}'."
            )

        vis_versions = []
        for resource_name in resource_names:
            if "gmod" in resource_name and "versioning" not in resource_name:
                stream_content = EmbeddedResource.get_decompressed_stream(
                    os.path.join(directory, resource_name)
                )
                gmod = GmodDto.parse_raw(stream_content)
                vis_versions.append(gmod.vis_version)

        return vis_versions
