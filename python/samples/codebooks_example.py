#!/usr/bin/env python3
"""Codebooks and Metadata Tags - Vista SDK Python.

This example demonstrates working with codebooks and metadata tags:
- Loading and exploring codebooks
- Creating standard and custom metadata tags
- Validating tag values
- Working with different tag types
"""

from vista_sdk.codebook_names import CodebookName
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


def main() -> None:  # noqa : C901
    """Demonstrate codebook operations."""
    print("=== Codebooks and Metadata Tags Example ===\n")

    # Initialize VIS
    vis = VIS()
    version = VisVersion.v3_4a
    codebooks = vis.get_codebooks(version)

    # 1. Exploring available codebooks
    print("1. Available Codebooks...")

    available_codebooks = [
        CodebookName.Quantity,
        CodebookName.Content,
        CodebookName.Position,
        CodebookName.State,
        CodebookName.Command,
        CodebookName.Type,
        CodebookName.Detail,
    ]

    for codebook_name in available_codebooks:
        codebook = codebooks[codebook_name]
        print(f"   {codebook_name.name}:")
        print(f"     → Standard values: {len(list(codebook.standard_values))}")
        print(f"     → Groups: {len(list(codebook.groups))}")
        print()

    # 2. Creating standard metadata tags
    print("2. Creating Standard Metadata Tags...")

    standard_tags = {
        "quantity": ["temperature", "pressure", "flow"],
        "position": ["centre", "inlet", "outlet"],
        "state": ["opened", "closed", "high", "low"],
        "content": ["cooling.water", "exhaust.gas", "fuel.oil"],
    }

    for tag_type, values in standard_tags.items():
        print(f"   {tag_type.title()} tags:")
        codebook_name = getattr(CodebookName, tag_type.title())
        codebook = codebooks[codebook_name]

        for value in values:
            try:
                tag = codebook.create_tag(value)
                print(f"     ✓ {value}: {tag} (custom: {tag.is_custom})")
            except Exception as e:
                print(f"     ✗ {value}: Failed - {e}")
        print()

    # 3. Working with custom tags
    print("3. Working with Custom Tags...")

    custom_examples = [
        (CodebookName.Quantity, "custom_temperature"),
        (CodebookName.Position, "custom_location"),
        (CodebookName.Content, "special.fluid"),
        (CodebookName.State, "partially_open"),
    ]

    for codebook_name, custom_value in custom_examples:
        codebook = codebooks[codebook_name]
        try:
            custom_tag = codebook.try_create_tag(custom_value)
            if custom_tag:
                print(
                    f"   ✓ {codebook_name.name}: {custom_tag} (custom: {custom_tag.is_custom})"  # noqa: E501
                )
            else:
                print(
                    f"   ✗ {codebook_name.name}: Failed to create tag for '{custom_value}'"  # noqa: E501
                )
        except Exception as e:
            print(f"   ✗ {codebook_name.name}: Exception - {e}")

    # 4. Tag validation
    print("\n4. Tag Validation...")

    validation_tests = [
        (CodebookName.Position, "centre", True),
        (CodebookName.Position, "invalid_position", False),
        (CodebookName.Quantity, "temperature", True),
        (CodebookName.Quantity, "nonexistent_quantity", False),
        (CodebookName.State, "opened", True),
        (CodebookName.State, "maybe_opened", False),
    ]

    for codebook_name, test_value, expected_valid in validation_tests:
        codebook = codebooks[codebook_name]
        is_valid = codebook.has_standard_value(test_value)
        status = "✓" if is_valid == expected_valid else "✗"
        print(
            f"   {status} {codebook_name.name}.{test_value}: "
            f"valid={is_valid} (expected: {expected_valid})"
        )

    # 5. Position validation (special case)
    print("\n5. Position Validation (Special Rules)...")

    position_codebook = codebooks[CodebookName.Position]
    position_tests = [
        "centre",
        "centre-1",
        "1-centre",  # Invalid order
        "port-starboard",  # Invalid grouping
        "phase.w.u",
        "outside-phase.w.u-10",
        "custom_position",
    ]

    for position in position_tests:
        try:
            validation_result = position_codebook.validate_position(position)
            print(f"   Position '{position}': {validation_result}")
        except Exception as e:
            print(f"   Position '{position}': Error - {e}")

    # 6. Exploring codebook content
    print("\n6. Exploring Codebook Content...")

    quantity_codebook = codebooks[CodebookName.Quantity]
    print("   Quantity codebook sample values:")

    # Show first 10 standard values
    values = list(quantity_codebook.standard_values)[:10]
    for value in values:
        print(f"     - {value}")

    print("   Quantity codebook sample groups:")
    groups = list(quantity_codebook.groups)[:5]
    for group in groups:
        print(f"     - {group}")

    print("\n=== Codebook operations completed! ===")


if __name__ == "__main__":
    main()
