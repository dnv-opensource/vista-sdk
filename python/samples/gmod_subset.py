"""Asset Model Example: Building a Digital Twin Structure from GmodPaths.

This example demonstrates one way to build an asset model (digital twin structure)
from a list of GmodPaths. This is the foundation for:
- Defining what equipment exists on your vessel
- Creating hierarchical visualizations
- Generating LocalIds for sensor data

The key insight is that a GmodPath like "411.1-1/C101/C101.31-1" implicitly
contains the entire hierarchy. GMOD validates that the path is structurally
valid, and we can build a tree by combining multiple paths.

Flow:
    Design-time:  GmodPaths (asset definition) → Asset Model → LocalIds
    Runtime:      LocalIds (from data) → Extract paths → Asset Model

This mirrors the TypeScript Pmod implementation used in frontend applications.
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field

from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis_version import VisVersion


@dataclass
class AssetNode:
    """A node in the asset model tree.

    Wraps a GmodNode with tree structure for visualization.
    Includes the full GmodPath for context when looking up nodes.
    """

    node: GmodNode
    path: GmodPath | None
    depth: int
    children: list[AssetNode]
    parent: AssetNode | None

    def __init__(self, node: GmodNode, path: GmodPath | None, depth: int) -> None:
        """Initialize an AssetNode."""
        self.node = node
        self.path = path
        self.depth = depth
        self.children = []
        self.parent = None
        # Common names are some time a combination of parent+child, so cache it here
        # Example: Component "C101 - Reciprocating internal combustion engine" becomes "C101 - Propulsion engine" in context of 411.1  # noqa: E501
        common_names = (
            [c[1] for c in path.get_common_names()] if path is not None else []
        )
        self.display_name = (
            common_names[-1]
            if common_names
            else node.metadata.common_name or node.metadata.name
        )

    def add_child(self, child: AssetNode) -> None:
        """Add a child node."""
        child.parent = self
        self.children.append(child)

    def to_dict(self) -> dict:
        """Convert to dictionary for JSON export."""
        return {
            "path": str(self.path),
            "code": self.node.code,
            "name": self.node.metadata.name,
            "commonName": self.node.metadata.common_name,
            "displayName": self.display_name,
            "category": self.node.metadata.category,
            "type": self.node.metadata.type,
            "location": str(self.node.location) if self.node.location else None,
            "children": [
                child.to_dict()
                for child in sorted(
                    self.children, key=lambda c: str(c.path) if c.path else ""
                )
            ],
        }

    def print_tree(self, indent: int = 0) -> None:
        """Print the tree structure to console."""
        prefix = "  " * indent
        connector = "├─" if indent > 0 else ""
        display_name = self.display_name
        location = f" [{self.node.location}]" if self.node.location else ""
        print(f"{prefix}{connector} {self.node.code}{location}: {display_name}")
        for child in sorted(self.children, key=lambda c: str(c.path) if c.path else ""):
            child.print_tree(indent + 1)


@dataclass
class AssetModel:
    """An asset model built from GmodPaths.

    This class builds a tree structure from a list of GmodPaths,
    similar to how Pmod works in the TypeScript implementation.

    Each unique full path becomes a node in the tree. Paths that
    share common prefixes will share parent nodes in the tree.

    The `nodes_by_code` property provides quick lookup of all instances
    of a particular node code (e.g., all engines, all cylinders).
    """

    vis_version: VisVersion
    root: AssetNode | None = None
    _node_map: dict[str, AssetNode] = field(default_factory=dict)
    _nodes_by_code: dict[str, list[AssetNode]] = field(default_factory=dict)

    @classmethod
    def from_paths(
        cls,
        vis_version: VisVersion,
        paths: list[GmodPath],
    ) -> AssetModel:
        """Create an asset model from a list of GmodPaths.

        Each path is expanded to include all intermediate nodes,
        building a complete tree structure.

        Args:
            vis_version: The VIS version for this model.
            paths: List of GmodPaths representing equipment on the asset.

        Returns:
            An AssetModel with all paths merged into a tree.
        """
        model = cls(vis_version=vis_version)

        for path in paths:
            model._add_path(path)

        return model

    @classmethod
    def from_path_strings(
        cls,
        vis_version: VisVersion,
        path_strings: list[str],
    ) -> AssetModel:
        """Create an asset model from path strings.

        Convenience method that parses path strings first.

        Args:
            vis_version: The VIS version for parsing and model.
            path_strings: List of path strings (e.g., "411.1-1/C101/C101.31-1").

        Returns:
            An AssetModel with all paths merged into a tree.
        """
        paths = []
        for path_str in path_strings:
            success, path = GmodPath.try_parse(path_str, vis_version)
            if success and path is not None:
                paths.append(path)
            else:
                print(f"  Warning: Could not parse path '{path_str}'")

        return cls.from_paths(vis_version, paths)

    def _add_path(self, path: GmodPath) -> None:
        """Add a single path to the model, creating all intermediate nodes."""
        full_path = path.get_full_path()

        for i, (depth, node) in enumerate(full_path):
            # Build the full path ID up to this point
            node_id = "/".join(str(full_path[j][1]) for j in range(i + 1))

            # Skip if already added
            if node_id in self._node_map:
                continue

            # Build the GmodPath for this node
            parents = [full_path[j][1] for j in range(i)]
            node_path = GmodPath(parents, node, skip_verify=True)

            # Create the asset node
            asset_node = AssetNode(
                node=node,
                path=node_path,
                depth=depth,
            )

            # Add to map
            self._node_map[node_id] = asset_node

            # Add to nodes_by_code index
            code = node.code
            if code not in self._nodes_by_code:
                self._nodes_by_code[code] = []
            self._nodes_by_code[code].append(asset_node)

            # Handle root node
            if depth == 0:
                if self.root is None:
                    self.root = asset_node
                continue

            # Find parent and link
            parent_id = "/".join(str(full_path[j][1]) for j in range(i))
            parent_node = self._node_map.get(parent_id)

            if parent_node:
                parent_node.add_child(asset_node)

    def get_node(self, node_id: str) -> AssetNode | None:
        """Get an asset node by its full path ID."""
        return self._node_map.get(node_id)

    @property
    def nodes_by_code(self) -> dict[str, list[AssetNode]]:
        """Get all nodes grouped by their code.

        Returns a dict mapping code -> list of AssetNodes.
        Useful for queries like "show me all engines (C101)".
        """
        return self._nodes_by_code

    def get_nodes_by_code(self, code: str) -> list[AssetNode]:
        """Get all asset nodes with a specific code."""
        return self._nodes_by_code.get(code, [])

    @property
    def node_count(self) -> int:
        """Get the total number of nodes in the model."""
        return len(self._node_map)

    @property
    def max_depth(self) -> int:
        """Get the maximum depth of the tree."""
        if not self._node_map:
            return 0
        return max(n.depth for n in self._node_map.values())

    def to_dict(self) -> dict:
        """Convert the entire tree to a dictionary."""
        if self.root is None:
            return {}
        return self.root.to_dict()

    def to_json(self, indent: int = 2) -> str:
        """Export the tree as JSON string."""
        return json.dumps(self.to_dict(), indent=indent)

    def print_tree(self) -> None:
        """Print the tree structure to console."""
        if self.root:
            self.root.print_tree()


# =============================================================================
# Examples
# =============================================================================


def example_dual_engine_vessel() -> None:
    """Example: Define a dual-engine vessel with cylinders.

    This demonstrates the primary use case: defining what equipment
    exists on your vessel by specifying GmodPaths.

    Each path like "411.1-1/C101/C101.31-1" represents:
    - Main Engine Function 1 (port) / Engine / Cylinder 1
    """
    print("=" * 70)
    print("Example 1: dual-engine Vessel Asset Model")
    print("=" * 70)

    vis_version = VisVersion.v3_4a

    # Define the equipment that exists on this vessel
    # Each path is a "leaf" item e.g. ISO19848 Annex C short path - the full hierarchy is implicit  # noqa: E501
    asset_paths = [
        # === Port Main Engine (411.1-P) with 6 cylinders ===
        "411.1-P/C101/C101.31-1",  # Port engine, cylinder 1
        "411.1-P/C101/C101.31-2",  # Port engine, cylinder 2
        "411.1-P/C101/C101.31-3",  # Port engine, cylinder 3
        "411.1-P/C101/C101.31-4",  # Port engine, cylinder 4
        "411.1-P/C101/C101.31-5",  # Port engine, cylinder 5
        "411.1-P/C101/C101.31-6",  # Port engine, cylinder 6
        "411.1-P/C101/C101.63/S206",  # Port engine, cooling system
        # === Starboard Main Engine (411.1-S) with 6 cylinders ===
        "411.1-S/C101/C101.31-1",  # Starboard engine, cylinder 1
        "411.1-S/C101/C101.31-2",  # Starboard engine, cylinder 2
        "411.1-S/C101/C101.31-3",  # Starboard engine, cylinder 3
        "411.1-S/C101/C101.31-4",  # Starboard engine, cylinder 4
        "411.1-S/C101/C101.31-5",  # Starboard engine, cylinder 5
        "411.1-S/C101/C101.31-6",  # Starboard engine, cylinder 6
        "411.1-S/C101/C101.63/S206",  # Starboard engine, cooling system
        # === Generator Sets ===
        "511.11-1/C101",  # Generator 1, diesel engine
        "511.11-2/C101",  # Generator 2, diesel engine
        # === Fuel System ===
        "621.21/S90",  # Fuel oil transfer system, piping
    ]

    print(f"\n  Defining {len(asset_paths)} equipment paths...")
    print("  (Each path implicitly includes all parent nodes)")

    # Build the asset model
    model = AssetModel.from_path_strings(vis_version, asset_paths)

    print(f"\n  Built model with {model.node_count} total nodes")
    print(f"  Maximum depth: {model.max_depth}")

    # Demonstrate nodes_by_code lookup - "show me all engines"
    print("\n  === Lookup: All Engines (C101) ===")
    engines = model.nodes_by_code.get("C101", [])
    print(f"  Found {len(engines)} instances of C101 (Internal combustion engine):")
    for engine in engines:
        print(f"    - Path: {engine.path}")

    # Demonstrate looking up all cylinders
    print("\n  === Lookup: All Cylinders (C101.31) ===")
    cylinders = model.nodes_by_code.get("C101.31", [])
    print(f"  Found {len(cylinders)} instances of C101.31 (Cylinder):")
    for cyl in cylinders[:6]:  # Show first 6
        print(f"    - Path: {cyl.path}")
    if len(cylinders) > 6:
        print(f"    ... and {len(cylinders) - 6} more")

    # Show available codes in the model
    print("\n  === All available codes in model ===")
    print(
        f"  {len(model.nodes_by_code)} unique codes: {sorted(model.nodes_by_code.keys())}"  # noqa: E501
    )

    print("\n  Asset Model Tree:")
    print()
    model.print_tree()


def example_json_export() -> None:
    """Example: Export model as JSON for visualization tools.

    The JSON format is suitable for tree visualization libraries
    like D3.js, vis.js, or React tree components.
    """
    print("\n" + "=" * 70)
    print("Example 3: JSON Export for Visualization")
    print("=" * 70)

    vis_version = VisVersion.v3_4a

    # Small example for readable JSON
    asset_paths = [
        "411.1/C101/C101.31-1",
        "411.1/C101/C101.31-2",
    ]

    model = AssetModel.from_path_strings(vis_version, asset_paths)

    print("\n  JSON output (for D3.js, etc.):")
    print()
    print(model.to_json(indent=2))


def example_from_local_ids() -> None:
    """Example: Build model from existing LocalIds.

    This demonstrates the runtime flow - when you already have
    LocalIds (e.g., from a DataChannelList or data source), you can
    extract the GmodPaths and build a model for visualization.

    This is how the TypeScript Pmod is typically used in frontends:
    LocalIds arrive from the backend, and the model is built on-the-fly.
    """
    print("\n" + "=" * 70)
    print("Example 2: Build Model from LocalIds (Runtime Flow)")
    print("=" * 70)

    from vista_sdk.local_id import LocalId

    vis_version = VisVersion.v3_4a

    # Simulate LocalIds coming from a DataChannelList or data source
    local_id_strings = [
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-3/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-2/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-2/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/511.11-1/C101/meta/qty-revolution",
    ]

    print(f"\n  Received {len(local_id_strings)} LocalIds from data source")

    # Parse LocalIds and extract GmodPaths
    paths: list[GmodPath] = []
    for lid_str in local_id_strings:
        local_id = LocalId.parse(lid_str)
        if local_id.primary_item:
            paths.append(local_id.primary_item)
        if local_id.secondary_item:
            paths.append(local_id.secondary_item)

    print(f"  Extracted {len(paths)} GmodPaths from LocalIds")

    # Build model from extracted paths
    model = AssetModel.from_paths(vis_version, paths)

    print(f"\n  Built model with {model.node_count} nodes")
    print(f"  Maximum depth: {model.max_depth}")

    print("\n  Asset Model Tree (derived from LocalIds):")
    print()
    model.print_tree()


def main() -> None:
    """Run all asset model examples."""
    print("\n" + "=" * 70)
    print("Asset Model Examples: Building Digital Twin Structures from GmodPaths")
    print("=" * 70)
    print()
    print("These examples demonstrate how to define what equipment exists")
    print("on your vessel using GmodPaths, then build a tree structure")
    print("for visualization and further processing (like LocalId generation).")

    example_dual_engine_vessel()
    example_from_local_ids()
    example_json_export()

    print("\n" + "=" * 70)
    print("All examples completed!")
    print("=" * 70)


if __name__ == "__main__":
    main()
