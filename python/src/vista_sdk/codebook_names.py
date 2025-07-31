"""This is temp."""

from enum import IntEnum


class CodebookName(IntEnum):
    """Integer enum for codebook name."""

    Quantity = 1
    Content = 2
    Calculation = 3
    State = 4
    Command = 5
    Type = 6
    FunctionalServices = 7
    MaintenanceCategory = 8
    ActivityType = 9
    Position = 10
    Detail = 11


class CodebookNames:
    """Helper methods for the codebook."""

    @staticmethod
    def from_prefix(prefix: str | None) -> CodebookName:  # noqa : C901
        """Convert from prefix to codebook name."""
        if prefix is None:
            raise ValueError("prefix cannot be None")

        match prefix:
            case "pos":
                return CodebookName.Position
            case "qty":
                return CodebookName.Quantity
            case "calc":
                return CodebookName.Calculation
            case "state":
                return CodebookName.State
            case "cnt":
                return CodebookName.Content
            case "cmd":
                return CodebookName.Command
            case "type":
                return CodebookName.Type
            case "funct.svc":
                return CodebookName.FunctionalServices
            case "maint.cat":
                return CodebookName.MaintenanceCategory
            case "act.type":
                return CodebookName.ActivityType
            case "detail":
                return CodebookName.Detail
            case _:
                raise ValueError(f"unknown prefix: {prefix}")

    @staticmethod
    def to_prefix(name: CodebookName) -> str:  # noqa : C901
        """Convert from codebook to Codebook prefix."""
        match name:
            case CodebookName.Position:
                return "pos"
            case CodebookName.Quantity:
                return "qty"
            case CodebookName.Calculation:
                return "calc"
            case CodebookName.State:
                return "state"
            case CodebookName.Content:
                return "cnt"
            case CodebookName.Command:
                return "cmd"
            case CodebookName.Type:
                return "type"
            case CodebookName.FunctionalServices:
                return "funct.svc"
            case CodebookName.MaintenanceCategory:
                return "maint.cat"
            case CodebookName.ActivityType:
                return "act.type"
            case CodebookName.Detail:
                return "detail"
            case _:
                raise ValueError(f"unknown codebook: {name}")
