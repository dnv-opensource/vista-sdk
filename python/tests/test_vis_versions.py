import os
import unittest
from vista_sdk.VisVersions import VisVersions, VisVersion, VisVersionExtension
from vista_sdk.SourceGenerator.VisVersionsGenerator import generate_vis_version_script

class TestVisVersions(unittest.TestCase):
    def test_to_version_string(self):
        builder = []
        result = VisVersionExtension.to_version_string(VisVersion.v3_4a, builder)
        self.assertEqual(result, "3-4a")
        self.assertIn("3-4a", builder)

    def test_to_string(self):
        builder = []
        result = VisVersionExtension.to_string(VisVersion.v3_5a, builder)
        self.assertEqual(result, "3-5a")
        self.assertIn("3-5a", builder)

    def test_is_valid(self):
        self.assertFalse(VisVersionExtension.is_valid("3-8a")) 

    def test_all_versions(self):
        versions = VisVersions.all_versions()
        self.assertIn(VisVersion.v3_7a, versions)
        self.assertEqual(len(versions), 4) 

    def test_try_parse(self):
        self.assertEqual(VisVersions.try_parse("3-4a"), VisVersion.v3_4a)
        with self.assertRaises(ValueError):
            VisVersions.try_parse("invalid-version")

    def test_parse(self):
        with self.assertRaises(ValueError):
            VisVersions.parse("invalid-version")

    def test_Vis_generation(self):
        root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        resources_dir = os.path.join(root_dir, "resources")
        output_file = os.path.join(root_dir, "python", "vista_sdk", "VisVersions.py")

        generate_vis_version_script(resources_dir, output_file)
        