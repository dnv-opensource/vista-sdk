"""Implementation of the MetadataTag class."""

from dataclasses import dataclass

from vista_sdk.codebook_names import CodebookName


@dataclass(frozen=True)
class MetadataTag:
    """A medata tag for a codebook entry."""

    name: CodebookName
    value: str
    is_custom: bool = False

    @property
    def prefix(self) -> str:
        """Get the prefix of the metadata tag."""
        return "~" if self.is_custom else "-"

    def __eq__(self, other: object) -> bool:
        """Check equality with another MetadataTag."""
        if not isinstance(other, MetadataTag):
            return False

        if self.name != other.name:
            raise ValueError(
                f"Cannot compare MetadataTag with different names: "
                f"{self.name} vs {other.name}"
            )

        return self.value == other.value

    def __hash__(self) -> int:
        """Get the hash of the MetadataTag."""
        return hash(self.value)

    def __str__(self) -> str:
        """Get the string representation of the MetadataTag."""
        return self.value

    def to_string(self, builder: list, separator: str = "/") -> None:
        """Append the string representation to the builder."""
        prefix = {
            CodebookName.Position: "pos",
            CodebookName.Quantity: "qty",
            CodebookName.Calculation: "calc",
            CodebookName.State: "state",
            CodebookName.Content: "cnt",
            CodebookName.Command: "cmd",
            CodebookName.Type: "type",
            CodebookName.FunctionalServices: "funct.svc",
            CodebookName.MaintenanceCategory: "maint.cat",
            CodebookName.ActivityType: "act.type",
            CodebookName.Detail: "detail",
        }.get(self.name)

        if prefix is None:
            raise ValueError("Unknown metadata tag:" + str({self.name}))

        builder.append(f"{prefix}{self.prefix}{self.value}{separator}")
