#!/usr/bin/env python3
"""Advanced Local ID Operations - Vista SDK Python.

This example demonstrates advanced Local ID operations:
- Building complex Local IDs with multiple components
- Error handling during parsing
- Working with custom tags
- Local ID validation and inspection
"""

from vista_sdk.codebook_names import CodebookName
from vista_sdk.gmod_path import GmodPath
from vista_sdk.local_id_builder import LocalIdBuilder
from vista_sdk.local_id_builder_parsing import LocalIdBuilderParsing
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


def main() -> None:  # noqa : C901
    """Demonstrate advanced Local ID operations."""
    print("=== Advanced Local ID Operations ===\n")

    # Initialize VIS
    vis = VIS()
    version = VisVersion.v3_4a
    codebooks = vis.get_codebooks(version)

    # 1. Building complex Local IDs
    print("1. Building Complex Local IDs...")

    # Primary and secondary items
    primary_path = GmodPath.parse("411.1/C101.31-2", arg=version)
    secondary_path = GmodPath.parse("411.1/C101.63/S206", arg=version)

    complex_local_id = (
        LocalIdBuilder.create(version)
        .with_primary_item(primary_path)
        .with_secondary_item(secondary_path)
        .with_metadata_tag(codebooks.create_tag(CodebookName.Quantity, "temperature"))
        .with_metadata_tag(codebooks.create_tag(CodebookName.Content, "exhaust.gas"))
        .with_metadata_tag(codebooks.create_tag(CodebookName.State, "high"))
        .with_metadata_tag(codebooks.create_tag(CodebookName.Position, "inlet"))
        .build()
    )

    print(f"   Complex Local ID: {complex_local_id}")
    print(f"   Has secondary item: {complex_local_id.secondary_item is not None}")
    print(f"   Number of metadata tags: {len(complex_local_id.metadata_tags)}")

    # 2. Working with custom tags
    print("\n2. Working with Custom Tags...")

    # Create custom tags (prefix with ~)
    custom_quantity = codebooks.try_create_tag(
        CodebookName.Quantity, "custom_temperature"
    )
    custom_position = codebooks.try_create_tag(CodebookName.Position, "custom_location")

    if custom_quantity and custom_position:
        custom_local_id = (
            LocalIdBuilder.create(version)
            .with_primary_item(primary_path)
            .with_metadata_tag(custom_quantity)
            .with_metadata_tag(custom_position)
            .build()
        )

        print(f"   Custom Local ID: {custom_local_id}")
        print(f"   Quantity tag is custom: {custom_local_id.quantity.is_custom}")  # type: ignore
        print(f"   Position tag is custom: {custom_local_id.position.is_custom}")  # type: ignore

    # 3. Error handling and validation
    print("\n3. Error Handling and Validation...")

    invalid_local_ids = [
        "/dnv-v2/vis-3-4a/invalid-path/meta/qty-temperature",
        "/invalid-naming-rule/vis-3-4a/411.1/meta/qty-temperature",
        "/dnv-v2/vis-3-4a/411.1/meta/qty-invalid_quantity",
        # Invalid position order
        "/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/pos-1-centre",
    ]

    for invalid_id in invalid_local_ids:
        print(f"\n   Testing invalid Local ID: {invalid_id}")

        try:
            success, errors, result = LocalIdBuilder.try_parse(invalid_id)

            if not success or result is None:
                print("     ✗ Parsing failed as expected")
                if errors.has_errors:
                    print("     Errors found:")
                    for error_type, message in errors:
                        print(f"       - {error_type}: {message}")
            else:
                print(f"     ⚠ Unexpectedly parsed successfully: {result}")

        except Exception as e:
            print(f"     ✗ Exception during parsing: {e}")

    # 4. Local ID inspection and analysis
    print("\n4. Local ID Inspection and Analysis...")

    valid_local_ids = [
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
    ]

    for local_id_str in valid_local_ids:
        print(f"\n   Analyzing: {local_id_str}")

        try:
            parser = LocalIdBuilderParsing()
            local_id = parser.parse(local_id_str).build()

            print("     ✓ Successfully parsed and built")
            print(f"     ✓ VIS Version: {local_id.vis_version}")
            print(f"     ✓ Has custom tags: {local_id.has_custom_tag}")
            print(f"     ✓ Primary item: {local_id.primary_item}")

            if local_id.secondary_item:
                print(f"     ✓ Secondary item: {local_id.secondary_item}")

            # Analyze each metadata tag
            print("     ✓ Metadata tags:")
            for tag in local_id.metadata_tags:
                print(
                    f"       - {tag.name.name}: {tag.value} (custom: {tag.is_custom})"
                )

        except Exception as e:
            print(f"     ✗ Failed to parse: {e}")

    # 5. Builder pattern variations
    print("\n5. Builder Pattern Variations...")

    # Using try_with methods for safe building
    builder = LocalIdBuilder.create(version)

    # Try adding components safely
    success, path_for_builder = GmodPath.try_parse("411.1/C101.31", version)
    if success and path_for_builder:
        builder = builder.with_primary_item(path_for_builder)

    # Try with valid tags
    temp_tag = codebooks.try_create_tag(CodebookName.Quantity, "temperature")
    if temp_tag:
        builder = builder.with_metadata_tag(temp_tag)

    exhaust_tag = codebooks.try_create_tag(CodebookName.Content, "exhaust.gas")
    if exhaust_tag:
        builder = builder.with_metadata_tag(exhaust_tag)

    # Try with potentially invalid values (this should fail)
    invalid_tag = codebooks.try_create_tag(
        CodebookName.Position, "invalid-position-format"
    )
    if invalid_tag:
        builder = builder.with_metadata_tag(invalid_tag)

    try:
        safe_local_id = builder.build()
        print(f"   Safe building result: {safe_local_id}")
    except Exception as e:
        print(f"   Safe building failed: {e}")

    # 6. Verbose mode demonstration
    print("\n6. Verbose Mode...")

    verbose_local_id = (
        LocalIdBuilder.create(version)
        .with_verbose_mode(True)
        .with_primary_item(primary_path)
        .with_metadata_tag(codebooks.create_tag(CodebookName.Quantity, "temperature"))
        .build()
    )

    regular_local_id = (
        LocalIdBuilder.create(version)
        .with_verbose_mode(False)
        .with_primary_item(primary_path)
        .with_metadata_tag(codebooks.create_tag(CodebookName.Quantity, "temperature"))
        .build()
    )

    print(f"   Verbose mode: {verbose_local_id}")
    print(f"   Regular mode: {regular_local_id}")

    print("\n=== Advanced operations completed! ===")


if __name__ == "__main__":
    main()
