from vista_sdk.VisVersions import VisVersion
from vista_sdk.VIS import VIS
import unittest

class TestVis:

    @staticmethod
    def get_vis() -> VIS:
        from vista_sdk.VIS import VIS 
        return VIS()
    
    
class TestVISSingleton(unittest.TestCase):

    def test_singleton_instance(self):
        vis_a = VIS().instance
        vis_b = VIS().instance
        vis_a.get_gmod(VisVersion.v3_7a)
        vis_b.get_gmod(VisVersion.v3_7a)
        vis_c = VIS()
        vis_c.get_gmod(VisVersion.v3_7a)
        
        self.assertIs(vis_a, vis_c, "VIS instances are not the same")
        