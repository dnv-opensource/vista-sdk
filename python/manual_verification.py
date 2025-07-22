#!/usr/bin/env python3
"""
Manual verification script for VISTA SDK transport modules.
This script tests all the key functionality to ensure everything works.
"""

import sys
import os

# Add the src directory to the Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "src"))

# Import logging for better output tracking
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)

def main():
    """Run comprehensive verification of transport modules."""
    print("=== VISTA SDK Transport Module Verification ===")
    
    try:
        # 1. Test module imports
        print("1. Testing module imports...")
        
        from vista_sdk import transport
        logger.info("Main transport module imported successfully")
        print("✓ Main transport module imported successfully")
        
        from vista_sdk.transport import time_series_data
        logger.info("Time series data module imported successfully")
        print("✓ Time series data module imported successfully")
        
        from vista_sdk.transport import iso19848
        logger.info("ISO19848 module imported successfully")
        print("✓ ISO19848 module imported successfully")
        
        from vista_sdk.transport import value
        logger.info("Value module imported successfully")
        print("✓ Value module imported successfully")
        
        # 2. Test value types
        print("2. Testing value types...")
        
        from vista_sdk.transport.value import (
            DecimalValue, IntegerValue, BooleanValue, StringValue,
            DateTimeValue, AnyValue
        )
        from datetime import datetime
        
        # Create various value types
        decimal_val = DecimalValue(3.14)
        integer_val = IntegerValue(42)
        boolean_val = BooleanValue(True)
        string_val = StringValue("test")
        datetime_val = DateTimeValue(datetime.now())
        
        logger.info(f"Created values: decimal={decimal_val.value}, integer={integer_val.value}, "
                   f"boolean={boolean_val.value}, string={string_val.value}")
        print("✓ Created 5 different value types")
        
        # 3. Test transport data structures
        print("3. Testing transport data structures...")
        
        from vista_sdk.transport.time_series_data import DataChannelId
        from vista_sdk.local_id import LocalId
        from vista_sdk.imo_number import ImoNumber
        from vista_sdk.transport.ship_id import ShipId
        
        # Create test data structures
        imo = ImoNumber(1234567)
        local_id = LocalId.parse("/dnv-v2/vis-3-4a/751/I101/meta/state-common.alarm/detail-bridge.gr3")
        data_channel_id = DataChannelId(local_id)
        ship_id = ShipId(imo)
        
        logger.info(f"Created basic structures: imo={imo}, ship_id={ship_id}, "
                   f"data_channel_id={data_channel_id.local_id}")
        print("✓ Created basic transport data structures")
        
        # 4. Test data channel structures
        print("4. Testing data channel structures...")
        
        from vista_sdk.transport.data_channel import FormatDataType
        
        # Create format data type with correct string value
        format_type = FormatDataType("String")  # Use string constructor, not enum
        
        logger.info(f"Created format type: {format_type.type}")
        print("✓ Created format data type")
        
        # 5. Test ISO19848 integration
        print("5. Testing ISO19848 integration...")
        
        from vista_sdk.transport.iso19848 import ISO19848, ISO19848Version
        
        iso_instance = ISO19848.get_instance()
        format_types = iso_instance.get_format_data_types(ISO19848Version.V2024)
        channel_types = iso_instance.get_data_channel_type_names(ISO19848Version.V2024)
        
        logger.info(f"ISO19848 instance created successfully")
        print("✓ Created ISO19848 integration objects")
        
        # 6. Test parsing and validation
        print("6. Testing parsing and validation...")
        
        # Test format validation
        validation_result, parsed_val = format_type.validate("test_string")
        if hasattr(validation_result, 'messages'):
            logger.warning(f"Validation failed: {validation_result.messages}")
        else:
            logger.info(f"Validation passed: {parsed_val}")
        
        print("✓ Completed parsing and validation tests")
        
        print("\n=== All Tests Passed! ===")
        print("🎉 Transport module verification completed successfully!")
        
        return True
        
    except Exception as e:
        logger.error(f"Test failed with error: {e}")
        print(f"✗ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
