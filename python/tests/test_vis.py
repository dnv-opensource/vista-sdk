"""Test for the VIS singleton class.

This test ensures that the VIS class behaves as a singleton and that
multiple calls to get_gmod return the same instance.
"""

import unittest

from vista_sdk.vis_version import VisVersion


class TestVis:
    """Test class for the VIS singleton."""

    @staticmethod
    def get_vis():  # noqa: ANN205
        """Get the singleton instance of VIS."""
        from vista_sdk.vis import VIS

        return VIS()


class TestVISSingleton(unittest.TestCase):
    """Test case for the VIS singleton instance."""

    def test_singleton_instance(self) -> None:
        """Test that multiple calls to VIS return the same instance."""
        vis = TestVis.get_vis()
        vis_a = vis.instance
        vis_b = vis.instance
        vis_a.get_gmod(VisVersion.v3_7a)
        vis_b.get_gmod(VisVersion.v3_7a)
        vis_c = vis
        vis_c.get_gmod(VisVersion.v3_7a)

        assert vis_a is vis_c, "VIS instances are not the same"
