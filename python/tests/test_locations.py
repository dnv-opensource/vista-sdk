import unittest
from vista_sdk.Locations import  LocationGroup
from vista_sdk.VisVersions import VisVersion
from vista_sdk.LocationBuilder import LocationBuilder
from vista_sdk.VIS import VIS

class TestLocations(unittest.TestCase):

    def setUp(self):
        self.vis = VIS()
        self.locations = self.vis.get_locations(VisVersion.v3_4a)

    def test_locations_loads(self):
        self.assertIsNotNone(self.locations)
        self.assertIsNotNone(self.locations.groups)

    def test_location_groups_properties(self):
        values = list(LocationGroup) 
        values_int = [value.value for value in values]
        self.assertEqual(len(values_int), len(set(values_int)))
        self.assertEqual(len(values_int), 5) 
        self.assertEqual(LocationGroup.NUMBER.value, 0)
        for i in range(len(values_int) - 1):
            self.assertEqual(values_int[i + 1], values_int[i] + 1)

    def test_locations(self):
        value = "some_string"
        expected_error_messages = ["error_message1", "error_message2"]
        success, string_parsed_location = self.locations.try_parse(value)
        success = self.locations.try_parse(value)
        self.verify(success, string_parsed_location, expected_error_messages)

    def verify(self, succeeded, parsed_location, _):
        if not succeeded:
            self.assertFalse(succeeded)
            self.assertIsNone(parsed_location)
        else:
            self.assertTrue(succeeded)
            self.assertIsNotNone(parsed_location)

    def test_location_parse_throwing(self):
        with self.assertRaises(ValueError):
            self.locations.parse(None)
        with self.assertRaises(ValueError):
            self.locations.parse('') 

    def test_location_builder(self):
        location_str = "11FIPU"
        location = self.locations.parse(location_str)
        builder = LocationBuilder.create(self.locations)


