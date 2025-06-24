"""Concurrent operation tests for Vista SDK."""

import asyncio
import time
from concurrent.futures import ThreadPoolExecutor

import pytest

from vista_sdk.gmod_path import GmodPath
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.asyncio
async def test_concurrent_path_conversions() -> None:
    """Test concurrent path conversions between versions."""
    vis = VIS().instance
    source_gmod = vis.get_gmod(VisVersion.v3_6a)
    paths = []

    # Collect paths for conversion
    source_gmod.traverse(
        lambda parents, node: (
            paths.append(GmodPath(list(parents), node, skip_verify=True)),
            TraversalHandlerResult.CONTINUE,
        )[1]
        if parents
        else TraversalHandlerResult.STOP
    )

    async def convert_path(path: GmodPath) -> GmodPath | None:
        return vis.convert_path(VisVersion.v3_5a, path, VisVersion.v3_6a)

    # Convert paths concurrently
    tasks = [
        convert_path(path) for path in paths[:100]
    ]  # Limit to 100 paths for testing
    results = await asyncio.gather(*tasks)

    assert all(result is not None for result in results)


def test_parallel_gmod_operations() -> None:
    """Test parallel Gmod operations using ThreadPoolExecutor."""
    start_time = time.perf_counter()
    max_time = 300  # Seconds
    vis = VIS().instance
    versions = [VisVersion.v3_5a, VisVersion.v3_6a, VisVersion.v3_7a, VisVersion.v3_8a]

    def load_and_traverse_gmod(version: VisVersion) -> int:
        gmod = vis.get_gmod(version)
        paths = []
        gmod.traverse(
            lambda parents, node: (
                paths.append(GmodPath(list(parents), node, skip_verify=True)),
                TraversalHandlerResult.CONTINUE,
            )[1]
            if parents
            else TraversalHandlerResult.STOP
        )
        return len(paths)

    with ThreadPoolExecutor(max_workers=3) as executor:
        results = list(executor.map(load_and_traverse_gmod, versions))

    end_time = time.perf_counter()
    total_time = end_time - start_time
    assert total_time < max_time, (
        f"Test spent longer time than expected. Time taken: {total_time}, expectet time: {max_time}"  # noqa : E501
    )
    assert all(count > 0 for count in results)


"""

@pytest.mark.skip("Codebooks not yet implemented")
@pytest.mark.asyncio
async def test_concurrent_resource_loading() -> None:
    vis = VIS().instance
    versions = [VisVersion.v3_5a, VisVersion.v3_6a, VisVersion.v3_7a, VisVersion.v3_8a]

    async def load_resources(version: VisVersion) -> bool:
        gmod = vis.get_gmod(version)
        codebooks = vis.get_codebooks(version)
        locations = vis.get_locations(version)
        return all([gmod, codebooks, locations])

    tasks = [load_resources(version) for version in versions]
    results = await asyncio.gather(*tasks)

    assert all(results)
    """
