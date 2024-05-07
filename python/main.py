from cgi import test
from src import VIS
from tests.test_vis_versions import TestVisVersions as test_vis_versions

if __name__ == '__main__':
    """run get_locations_dto in VIS.py"""
    vis = VIS.VIS.instance()
    vis.get_locations_dto(VIS.VisVersion.v3_4a)
    print("done")
