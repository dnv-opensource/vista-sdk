"""Test for the VIS singleton class."""

import pytest

from vista_sdk.vis_version import VisVersion


@pytest.fixture
def vis_instance():  # noqa: ANN201
    """Fixture to provide a singleton instance of VIS."""
    from vista_sdk.vis import VIS

    return VIS()


@pytest.fixture
def source_gmod(vis_instance):  # noqa: ANN201, ANN001
    """Fixture to provide the source GMod for version v3_4a."""
    return vis_instance.get_gmod(VisVersion.v3_4a)


@pytest.fixture
def target_gmod(vis_instance):  # noqa: ANN201, ANN001
    """Fixture to provide the target GMod for version v3_5a."""
    return vis_instance.get_gmod(VisVersion.v3_5a)
