import sys
import os
from typing import List
import unittest

def setUp():
    bindings_path = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
    sys.path.append(bindings_path)

setUp()
import bindings

class TraversalCallbackImpl(bindings.TraversalCallback):
    def handler(self, __, _):
        return bindings.TraversalHandlerResult.CONTINUE


class GmodTestCase(unittest.TestCase):
    def setUp(self) -> None:
        self.version = bindings.VisVersion.V3_7A
        return super().setUp()

    def test_gmod_apis(self):
        gmod = bindings.Vis.instance().get_gmod(version=self.version)
        root_node = gmod.root_node()
        root_code = root_node.code()

        code = "C101"
        node = gmod.get_node(code)
        parents = gmod.get_parents(node)

        callback = TraversalCallbackImpl()
        finished = gmod.traverse(callback)
        self.assertTrue(finished)

        self.assertEqual(self.version, gmod.version())
        self.assertIsInstance(root_node, bindings.GmodNode)
        self.assertEqual("VE", root_code)
        self.assertEqual(code, node.code())
        self.assertNotEqual(len(parents), 0)

    def test_gmod_apis_statically(self):
        vis = bindings.Vis.instance()
        gmod = bindings.Vis.get_gmod(vis, self.version)
        root_node = bindings.Gmod.root_node(gmod)
        code = bindings.GmodNode.code(root_node)

        self.assertEqual(self.version, gmod.version())
        self.assertEqual(root_node.code(), code)

    def test_extension_classes(self):
        visVersionExtension = bindings.VisVersionExtensions()
        visVersions = bindings.VisVersions()

        version = self.version
        versionStr = visVersionExtension.to_version_string(version=version)
        parsed = visVersions.parse(versionStr)

        self.assertEqual("3-7a", versionStr)
        self.assertEqual(version, parsed)


    def test_false_constructors(self):
        # Just to ensure that the usage of the constructors throws errors
        self.assertRaises(BaseException, lambda _: bindings.Vis().get_gmod(self.version) )
        self.assertRaises(BaseException, lambda _: bindings.Gmod().version() )
        self.assertRaises(BaseException, lambda _: bindings.GmodNode().code() )

if __name__ == '__main__':
    unittest.main()


