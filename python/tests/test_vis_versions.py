import unittest
# from src.VisVersions import VisVersion, VisVersionExtension, VisVersions
from src.VisVersions import VisVersions, VisVersion, VisVersionExtension

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
        self.assertTrue(VisVersionExtension.is_valid(VisVersion.v3_6a))
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

if __name__ == '__main__':
    unittest.main()
