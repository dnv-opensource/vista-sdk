"""Test for the VIS singleton class."""

import pytest
from src.vista_sdk import vis, vis_version
from src.vista_sdk.vis import Gmod


@pytest.fixture
def vis_instance() -> vis.VIS:
    """Fixture to provide a singleton instance of VIS."""
    return vis.VIS.instance


@pytest.fixture
def source_gmod(vis_instance: vis.VIS) -> Gmod:
    """Fixture to provide the source GMod for version v3_4a."""
    return vis_instance.get_gmod(vis_version.VisVersion.v3_4a)


@pytest.fixture
def target_gmod(vis_instance: vis.VIS) -> Gmod:
    """Fixture to provide the target GMod for version v3_5a."""
    return vis_instance.get_gmod(vis_version.VisVersion.v3_5a)
