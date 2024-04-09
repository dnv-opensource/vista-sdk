import sys
import os
import unittest

def setUp():
    bindings_path = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
    sys.path.append(bindings_path)

setUp()
import bindings

class VisTestCase(unittest.TestCase):
    def test_get_vis(self):
        vis = bindings.Vis.instance()
        self.assertIsNotNone(vis)


if __name__ == '__main__':
    unittest.main()
