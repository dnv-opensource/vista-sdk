"""LocalIdBuilder for constructing LocalId objects."""

from __future__ import annotations

from typing import TYPE_CHECKING, ClassVar, TypeVar

from vista_sdk.codebook_names import CodebookName, CodebookNames
from vista_sdk.gmod_path import GmodPath
from vista_sdk.local_id_items import LocalIdItems
from vista_sdk.metadata_tag import MetadataTag
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.vis_version import VisVersion, VisVersions

# Forward references to avoid circular imports
T = TypeVar("T")
if TYPE_CHECKING:
    from vista_sdk.local_id import LocalId


class LocalIdBuilder:
    """Builder class for constructing LocalId objects with immutable semantics."""

    NAMING_RULE: ClassVar[str] = "dnv-v2"
    USED_CODEBOOKS: ClassVar[list[CodebookName]] = [
        CodebookName.Quantity,
        CodebookName.Content,
        CodebookName.State,
        CodebookName.Command,
        CodebookName.FunctionalServices,
        CodebookName.MaintenanceCategory,
        CodebookName.ActivityType,
        CodebookName.Position,
        CodebookName.Detail,
    ]

    def __init__(self) -> None:
        """Initialize a new LocalIdBuilder."""
        self._vis_version: VisVersion | None = None
        self._verbose_mode: bool = False
        self._items: LocalIdItems = LocalIdItems()
        self._quantity: MetadataTag | None = None
        self._content: MetadataTag | None = None
        self._calculation: MetadataTag | None = None
        self._state: MetadataTag | None = None
        self._command: MetadataTag | None = None
        self._type: MetadataTag | None = None
        self._position: MetadataTag | None = None
        self._detail: MetadataTag | None = None

    def _copy_with(self, **kwargs) -> LocalIdBuilder:  # noqa : ANN003
        """Create a copy of this builder with the specified attributes changed."""
        new_builder = LocalIdBuilder()
        new_builder._vis_version = kwargs.get("vis_version", self._vis_version)
        new_builder._verbose_mode = kwargs.get("verbose_mode", self._verbose_mode)
        new_builder._items = kwargs.get("items", self._items)
        new_builder._quantity = kwargs.get("quantity", self._quantity)
        new_builder._content = kwargs.get("content", self._content)
        new_builder._calculation = kwargs.get("calculation", self._calculation)
        new_builder._state = kwargs.get("state", self._state)
        new_builder._command = kwargs.get("command", self._command)
        new_builder._type = kwargs.get("type", self._type)
        new_builder._position = kwargs.get("position", self._position)
        new_builder._detail = kwargs.get("detail", self._detail)
        return new_builder

    @property
    def vis_version(self) -> VisVersion | None:
        """Get the VIS version."""
        return self._vis_version

    @property
    def verbose_mode(self) -> bool:
        """Get the verbose mode flag."""
        return self._verbose_mode

    @property
    def primary_item(self) -> GmodPath | None:
        """Get the primary item."""
        return self._items.primary_item

    @property
    def secondary_item(self) -> GmodPath | None:
        """Get the secondary item."""
        return self._items.secondary_item

    @property
    def quantity(self) -> MetadataTag | None:
        """Get the quantity metadata tag."""
        return self._quantity

    @property
    def content(self) -> MetadataTag | None:
        """Get the content metadata tag."""
        return self._content

    @property
    def calculation(self) -> MetadataTag | None:
        """Get the calculation metadata tag."""
        return self._calculation

    @property
    def state(self) -> MetadataTag | None:
        """Get the state metadata tag."""
        return self._state

    @property
    def command(self) -> MetadataTag | None:
        """Get the command metadata tag."""
        return self._command

    @property
    def type(self) -> MetadataTag | None:
        """Get the type metadata tag."""
        return self._type

    @property
    def position(self) -> MetadataTag | None:
        """Get the position metadata tag."""
        return self._position

    @property
    def detail(self) -> MetadataTag | None:
        """Get the detail metadata tag."""
        return self._detail

    def with_vis_version(self, vis_version: str | VisVersion) -> LocalIdBuilder:
        """Set the VIS version."""
        builder = self.try_with_vis_version(vis_version)
        if builder is None:
            raise ValueError(f"Failed to parse VIS version: {vis_version}")
        return builder

    def try_with_vis_version(
        self, vis_version: str | VisVersion | None
    ) -> LocalIdBuilder:
        """Try to set the VIS version."""
        if isinstance(vis_version, str):
            try:
                version_obj = VisVersions.parse(vis_version)
                return self.try_with_vis_version(version_obj)
            except ValueError:
                return self

        if vis_version is None:
            return self

        return self._copy_with(vis_version=vis_version)

    def without_vis_version(self) -> LocalIdBuilder:
        """Remove the VIS version."""
        return self._copy_with(vis_version=None)

    def with_verbose_mode(self, verbose_mode: bool) -> LocalIdBuilder:
        """Set the verbose mode flag."""
        return self._copy_with(verbose_mode=verbose_mode)

    def with_primary_item(self, item: GmodPath) -> LocalIdBuilder:
        """Set the primary item."""
        builder = self.try_with_primary_item(item)
        if builder is None:
            raise ValueError(f"Invalid primary item: {item}")
        return builder

    def try_with_primary_item(self, item: GmodPath | None) -> LocalIdBuilder:
        """Try to set the primary item."""
        if item is None:
            return self

        new_items = LocalIdItems(
            primary_item=item, secondary_item=self._items.secondary_item
        )
        return self._copy_with(items=new_items)

    def without_primary_item(self) -> LocalIdBuilder:
        """Remove the primary item."""
        new_items = LocalIdItems(
            primary_item=None, secondary_item=self._items.secondary_item
        )
        return self._copy_with(items=new_items)

    def with_secondary_item(self, item: GmodPath) -> LocalIdBuilder:
        """Set the secondary item."""
        builder = self.try_with_secondary_item(item)
        if builder is None:
            raise ValueError(f"Invalid secondary item: {item}")
        return builder

    def try_with_secondary_item(self, item: GmodPath | None) -> LocalIdBuilder:
        """Try to set the secondary item."""
        if item is None:
            return self

        new_items = LocalIdItems(
            primary_item=self._items.primary_item, secondary_item=item
        )
        return self._copy_with(items=new_items)

    def without_secondary_item(self) -> LocalIdBuilder:
        """Remove the secondary item."""
        new_items = LocalIdItems(
            primary_item=self._items.primary_item, secondary_item=None
        )
        return self._copy_with(items=new_items)

    def with_metadata_tag(self, metadata_tag: MetadataTag) -> LocalIdBuilder:
        """Set a metadata tag."""
        builder = self.try_with_metadata_tag(metadata_tag)
        if builder is None:
            raise ValueError(f"Invalid metadata tag: {metadata_tag}")
        return builder

    def try_with_metadata_tag(self, metadata_tag: MetadataTag | None) -> LocalIdBuilder:
        """Try to set a metadata tag."""
        if metadata_tag is None:
            return self

        if metadata_tag.name == CodebookName.Quantity:
            return self._with_quantity(metadata_tag)
        if metadata_tag.name == CodebookName.Content:
            return self._with_content(metadata_tag)
        if metadata_tag.name == CodebookName.Calculation:
            return self._with_calculation(metadata_tag)
        if metadata_tag.name == CodebookName.State:
            return self._with_state(metadata_tag)
        if metadata_tag.name == CodebookName.Command:
            return self._with_command(metadata_tag)
        if metadata_tag.name == CodebookName.Type:
            return self._with_type(metadata_tag)
        if metadata_tag.name == CodebookName.Position:
            return self._with_position(metadata_tag)
        if metadata_tag.name == CodebookName.Detail:
            return self._with_detail(metadata_tag)

        return self

    def without_metadata_tag(self, name: CodebookName) -> LocalIdBuilder:
        """Remove a metadata tag."""
        if name == CodebookName.Quantity:
            return self.without_quantity()
        if name == CodebookName.Content:
            return self.without_content()
        if name == CodebookName.Calculation:
            return self.without_calculation()  # Note: This typo matches the C# code
        if name == CodebookName.State:
            return self.without_state()
        if name == CodebookName.Command:
            return self.without_command()
        if name == CodebookName.Type:
            return self.without_type()
        if name == CodebookName.Position:
            return self.without_position()
        if name == CodebookName.Detail:
            return self.without_detail()

        return self

    def without_quantity(self) -> LocalIdBuilder:
        """Remove the quantity metadata tag."""
        return self._copy_with(quantity=None)

    def without_content(self) -> LocalIdBuilder:
        """Remove the content metadata tag."""
        return self._copy_with(content=None)

    def without_calculation(self) -> LocalIdBuilder:
        """Remove the calculation metadata tag."""
        return self._copy_with(calculation=None)

    def without_state(self) -> LocalIdBuilder:
        """Remove the state metadata tag."""
        return self._copy_with(state=None)

    def without_command(self) -> LocalIdBuilder:
        """Remove the command metadata tag."""
        return self._copy_with(command=None)

    def without_type(self) -> LocalIdBuilder:
        """Remove the type metadata tag."""
        return self._copy_with(type=None)

    def without_position(self) -> LocalIdBuilder:
        """Remove the position metadata tag."""
        return self._copy_with(position=None)

    def without_detail(self) -> LocalIdBuilder:
        """Remove the detail metadata tag."""
        return self._copy_with(detail=None)

    def _with_quantity(self, quantity: MetadataTag) -> LocalIdBuilder:
        """Set the quantity metadata tag."""
        return self._copy_with(quantity=quantity)

    def _with_content(self, content: MetadataTag) -> LocalIdBuilder:
        """Set the content metadata tag."""
        return self._copy_with(content=content)

    def _with_calculation(self, calculation: MetadataTag) -> LocalIdBuilder:
        """Set the calculation metadata tag."""
        return self._copy_with(calculation=calculation)

    def _with_state(self, state: MetadataTag) -> LocalIdBuilder:
        """Set the state metadata tag."""
        return self._copy_with(state=state)

    def _with_command(self, command: MetadataTag) -> LocalIdBuilder:
        """Set the command metadata tag."""
        return self._copy_with(command=command)

    def _with_type(self, type_tag: MetadataTag) -> LocalIdBuilder:
        """Set the type metadata tag."""
        return self._copy_with(type=type_tag)

    def _with_position(self, position: MetadataTag) -> LocalIdBuilder:
        """Set the position metadata tag."""
        return self._copy_with(position=position)

    def _with_detail(self, detail: MetadataTag) -> LocalIdBuilder:
        """Set the detail metadata tag."""
        return self._copy_with(detail=detail)

    @staticmethod
    def create(version: VisVersion) -> LocalIdBuilder:
        """Create a new LocalIdBuilder with the given VIS version."""
        return LocalIdBuilder().with_vis_version(version)

    def build(self) -> LocalId:
        """Build a LocalId from this builder."""
        if self.is_empty:
            raise ValueError("Cannot build a LocalId from an empty LocalIdBuilder")
        if not self.is_valid:
            raise ValueError("Cannot build a LocalId from an invalid LocalIdBuilder")

        # Import here to avoid circular import
        from vista_sdk.local_id import LocalId

        return LocalId(self)

    @staticmethod
    def parse(local_id_str: str) -> LocalIdBuilder:
        """Parse a string into a LocalIdBuilder.

        Args:
            local_id_str: The string to parse

        Returns:
            The resulting LocalIdBuilder
        """
        from vista_sdk.local_id_builder_parsing import LocalIdBuilderParsing

        return LocalIdBuilderParsing().parse(local_id_str)

    @staticmethod
    def try_parse(
        local_id_str: str,
    ) -> tuple[bool, ParsingErrors, LocalIdBuilder | None]:
        """Attempt to parse a string into a LocalIdBuilder.

        Args:
            local_id_str: The string to parse

        Returns:
            A tuple containing a boolean indicating success,
            any parsing errors, and the resulting LocalIdBuilder
        """
        from vista_sdk.local_id_builder_parsing import LocalIdBuilderParsing

        return LocalIdBuilderParsing().try_parse(local_id_str)

    @property
    def has_custom_tag(self) -> bool:
        """Check if this builder has any custom tags."""
        return (
            (self._quantity is not None and self._quantity.is_custom)
            or (self._calculation is not None and self._calculation.is_custom)
            or (self._content is not None and self._content.is_custom)
            or (self._position is not None and self._position.is_custom)
            or (self._state is not None and self._state.is_custom)
            or (self._command is not None and self._command.is_custom)
            or (self._type is not None and self._type.is_custom)
            or (self._detail is not None and self._detail.is_custom)
        )

    @property
    def is_valid(self) -> bool:
        """Check if this builder is valid."""
        return (
            self._vis_version is not None
            and self._items.primary_item is not None
            and (
                self._quantity is not None
                or self._calculation is not None
                or self._content is not None
                or self._position is not None
                or self._state is not None
                or self._command is not None
                or self._type is not None
                or self._detail is not None
            )
        )

    @property
    def is_empty(self) -> bool:
        """Check if this builder is empty."""
        return (
            self._items.primary_item is None
            and self._items.secondary_item is None
            and self.is_empty_metadata
        )

    @property
    def is_empty_metadata(self) -> bool:
        """Check if this builder has no metadata tags."""
        return (
            self._quantity is None
            and self._calculation is None
            and self._content is None
            and self._position is None
            and self._state is None
            and self._command is None
            and self._type is None
            and self._detail is None
        )

    @property
    def metadata_tags(self) -> list[MetadataTag]:
        """Get the metadata tags."""
        tags = []
        for tag in [
            self._quantity,
            self._calculation,
            self._content,
            self._position,
            self._state,
            self._command,
            self._type,
            self._detail,
        ]:
            if tag is not None:
                tags.append(tag)
        return tags

    def __eq__(self, other: object) -> bool:
        """Check if this builder is equal to another object."""
        if not isinstance(other, LocalIdBuilder):
            return False

        if self._vis_version != other._vis_version:
            raise ValueError("Cannot compare LocalIds from different VIS versions")

        return (
            self._items.primary_item == other._items.primary_item
            and self._items.secondary_item == other._items.secondary_item
            and self._quantity == other._quantity
            and self._calculation == other._calculation
            and self._content == other._content
            and self._position == other._position
            and self._state == other._state
            and self._command == other._command
            and self._type == other._type
            and self._detail == other._detail
        )

    def __hash__(self) -> int:
        """Get the hash of this builder."""
        return hash(
            (
                self._items.primary_item,
                self._items.secondary_item,
                self._quantity,
                self._calculation,
                self._content,
                self._position,
                self._state,
                self._command,
                self._type,
                self._detail,
            )
        )

    def to_string(self, builder: list[str]) -> None:
        """Convert this builder to a string."""
        if self._vis_version is None:
            raise ValueError("No VIS version configured on LocalId")

        naming_rule = f"/{self.NAMING_RULE}/"

        builder.append(naming_rule)

        builder.append("vis-")
        builder.append(str(self._vis_version))
        builder.append("/")

        self._items.append(builder, self._verbose_mode)

        builder.append("meta/")

        # Order of metadata tags matters - should match the naming rule/standard
        self._append_meta(builder, self._quantity)
        self._append_meta(builder, self._content)
        self._append_meta(builder, self._calculation)
        self._append_meta(builder, self._state)
        self._append_meta(builder, self._command)
        self._append_meta(builder, self._type)
        self._append_meta(builder, self._position)
        self._append_meta(builder, self._detail)

        # Remove trailing slash if present
        if builder and builder[-1].endswith("/"):
            builder[-1] = builder[-1][:-1]

    @staticmethod
    def _append_meta(builder: list[str], tag: MetadataTag | None) -> None:
        """Append a metadata tag to the string builder."""
        if tag is not None and isinstance(tag, MetadataTag):
            prefix = CodebookNames.to_prefix(tag.name)
            builder.append(f"{prefix}{tag.prefix}{tag.value}/")

    def __str__(self) -> str:
        """Get the string representation of this builder."""
        builder: list[str] = []
        self.to_string(builder)
        return "".join(builder)

    def __repr__(self) -> str:
        """Get the official string representation of this builder."""
        return f"LocalIdBuilder({self!s})"
