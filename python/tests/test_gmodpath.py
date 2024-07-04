from operator import contains
from typing import List, Optional
import unittest

from pydantic import ValidationError

from vista_sdk.GmodPath import GmodPath
from vista_sdk.VIS import VIS
from .test_vis import TestVis
from vista_sdk.VisVersions import VisVersion, VisVersionExtension, VisVersions
from vista_sdk.Client import Client
from .testdata import IndividualizableSetData, TestData
from parameterized import parameterized


class TestGmodPath(unittest.TestCase):

    def setUp(self):
        try:
            self.gmod_test_data = TestData.get_gmodpath_data("GmodPaths")
            self.test_individualizable_sets_data = TestData.get_Individualizable_sets_data("IndividualizableSets")
        except ValidationError as e:
            raise Exception("Couldn't load test data") from e
        self.vis = TestVis.get_vis()
        Client.get_gmod(VisVersionExtension.to_version_string(VisVersion.v3_4a))


    def test_gmodpath_parse(self):
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path : str = item.path
                path = GmodPath.try_parse(input_path, vis_version)
                self.assertTrue(path[0])
                self.assertIsNotNone(path[1])
                self.assertEqual(input_path, path[1].__str__())

    def test_gmodpath_parse_invalid(self):
        for item in self.gmod_test_data.invalid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path : str = item.path

                vis : VIS = self.vis

                path = GmodPath.try_parse(input_path, vis_version)
                self.assertFalse(path[0])
                self.assertIsNone(path[1])

  

    def test_get_full_path(self):
        vis = self.vis

        path_str = "411.1/C101.72/I101"
        expectation = {
            0: "VE",
            1: "400a",
            2: "410",
            3: "411",
            4: "411i",
            5: "411.1",
            6: "CS1",
            7: "C101",
            8: "C101.7",
            9: "C101.72",
            10: "I101",
        }

        seen = set()
        path = GmodPath.parse(path_str, arg=VisVersion.v3_4a)

        for depth, node in path.get_full_path():
            if depth in seen:
                self.fail("Got same depth twice")
            seen.add(depth)
            if len(seen) == 1:
                self.assertEqual(0, depth)
            self.assertEqual(expectation[depth], node.code)

        self.assertTrue(sorted(expectation.keys()) == sorted(seen))


    def test_get_full_path_from(self):
        vis = self.vis

        path_str = "411.1/C101.72/I101"
        expectation = {
            4: "411i",
            5: "411.1",
            6: "CS1",
            7: "C101",
            8: "C101.7",
            9: "C101.72",
            10: "I101",
        }

        seen = set()
        path = GmodPath.parse(path_str, arg=VisVersion.v3_4a)

        for depth, node in path.get_full_path_from(4):
            if depth in seen:
                self.fail("Got same depth twice")
            seen.add(depth)
            if len(seen) == 1:
                self.assertEqual(4, depth)
            self.assertEqual(expectation[depth], node.code)

        self.assertTrue(sorted(expectation.keys()) == sorted(seen))

  
    def test_full_path_parsing(self):
        version = VisVersion.v3_4a
        short_path_strs : List[str] = ["612.21-1/C701.13/S93"]
        expected_full_path_strs : List[str] = [ 
                                  "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93"]
        for i in range(len(short_path_strs)):
            path = GmodPath.parse(short_path_strs[i], arg=version)
            full_string = path.to_full_path_string()
            self.assertEqual(expected_full_path_strs[i], full_string)

            parsed, parsed_path = GmodPath.try_parse_full_path(full_string, arg=version)
            self.assertIsNotNone(parsed_path)
            self.assertTrue(parsed)
            self.assertEqual(path, parsed_path)
            self.assertEqual(full_string, path.to_full_path_string())
            self.assertEqual(full_string, parsed_path.to_full_path_string())
            self.assertEqual(short_path_strs[i], str(path))
            self.assertEqual(short_path_strs[i], str(parsed_path))

            parsed_path = GmodPath.parse_full_path(full_string, version)
            self.assertIsNotNone(parsed_path)
            self.assertEqual(path, parsed_path)
            self.assertEqual(full_string, path.to_full_path_string())
            self.assertEqual(full_string, parsed_path.to_full_path_string())
            self.assertEqual(short_path_strs[i], str(path))
            self.assertEqual(short_path_strs[i], str(parsed_path))
    

    def test_individualizable_sets(self):
        for item in self.test_individualizable_sets_data.data:
            with self.subTest(item=item):
                is_full_path : bool = item.is_full_path
                version = VisVersions.parse(item.vis_version)
                gmod = self.vis.get_gmod(version)

                if item.expected is None:
                    result = gmod.try_parse_from_full_path(item.path) if is_full_path else gmod.try_parse_path(item.path)
                    self.assertFalse(result[0])
                    self.assertIsNone(result[1])
                    return
                test = (item.is_full_path)
                path = gmod.parse_from_full_path(item.path) if is_full_path else gmod.parse_path(item.path)
                sets = path.individualizable_sets
                self.assertEqual(len(item.expected), len(sets))
                for i in range(len(item.expected)):
                    self.assertEqual(item.expected[i], [n.code for n in sets[i].nodes])

    def test_individualizable_sets_full_path(self):
        for item in self.test_individualizable_sets_data.data:
            with self.subTest(item=item):
                is_full_path : bool = item.is_full_path
                version = VisVersions.parse(item.vis_version)
                gmod = self.vis.get_gmod(version)

                if is_full_path:
                    return  

                if item.expected is None:
                    result, parsed = gmod.try_parse_path(item.path)
                    self.assertFalse(result)
                    self.assertIsNone(parsed)
                    return

                path = GmodPath.parse_full_path(gmod.parse_path(item.path).to_full_path_string(), version)
                sets = path.individualizable_sets
                self.assertEqual(len(item.expected), len(sets))
                for i in range(len(item.expected)):
                    self.assertEqual(item.expected[i], [node.code for node in sets[i].nodes])


    def test_gmod_path_does_not_individualize(self):
        version = VisVersion.v3_7a
        gmod = self.vis.get_gmod(version)
        parsed, path = gmod.try_parse_path("500a-1")
        self.assertFalse(parsed)
        self.assertIsNone(path)


    def test_to_full_path_string(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)
        path = gmod.parse_path("511.11-1/C101.663i-1/C663")
        self.assertEqual(
            "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663",
            path.to_full_path_string()
        )

        path = gmod.parse_path("846/G203.32-2/S110.2-1/E31")
        test = path.to_full_path_string()

        self.assertEqual(
            "VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31",
            path.to_full_path_string()
        )

    def test_valid_gmod_path_individualizable_sets(self):
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path = item.path

                gmod = self.vis.get_gmod(vis_version=vis_version)
                path = gmod.parse_path(input_path)
                sets = path.individualizable_sets

                unique_codes = set()
                for individual_set in sets:
                    for node in individual_set.nodes:
                        unique_codes.add(node.code)
                        self.assertTrue(node.is_individualizable)


    def test_valid_gmod_path_individualizable_sets_full_path(self):
       for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path = item.path

                gmod = self.vis.get_gmod(vis_version=vis_version)

                full_path_str = gmod.parse_path(input_path).to_full_path_string()
                path = GmodPath.parse_full_path(full_path_str, vis_version)
                sets = path.individualizable_sets

                unique_codes = set()
                for individual_set in sets:
                    for node in individual_set.nodes:
                        unique_codes.add(node.code)
                        self.assertTrue(node.is_individualizable)

    def test_common_names(self):
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path = item.path

                gmod = self.vis.get_gmod(vis_version=vis_version)
                path = gmod.parse_path(input_path)
                sets = path.individualizable_sets

                unique_codes = set()
                for individual_set in sets:
                    for node in individual_set.nodes:
                        unique_codes.add(node.code)
                        self.assertTrue(node.is_individualizable)
                        self.assertTrue(node.metadata.common_name is not None)
                        self.assertTrue(node.metadata.common_name != "")

    
    def test_verbose_path(self):
        path_strs = ["411.1-1/C101.71/I101", "411.1/C102.321/C502", "1000.1/F401.2"]
        target_strs = ["Propulsion engine 1/Control monitoring and alarm system", 
                       "Propulsion steam turbine/Intermediate pressure turbine rotor blade arrangement",
                       "Cargo data/Deadweight carried"]
        for(i, path_str) in enumerate(path_strs):
            with self.subTest(path_str=path_str):
                path = GmodPath.parse(path_str, arg=VisVersion.v3_7a)
                verbose_path = path.to_verbose_string()
                test = verbose_path
                self.assertEqual(target_strs[i], verbose_path)

    