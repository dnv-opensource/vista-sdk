from vista_sdk.internal.LocationParsingErrorBuilder import LocationValidationResult


class ParsingErrors:
    def __init__(self, errors=None):
        if errors is None:
            errors = []
        self._errors = errors

    @property
    def has_errors(self) -> bool:
        return len(self._errors) > 0

    def has_error_type(self, type : LocationValidationResult) -> bool:
        return any(e[0] == type for e in self._errors)

    def __str__(self) -> str:
        if not self.has_errors:
            return "Success"
        
        error_messages = ["Parsing errors:"]
        for type, message in self._errors:
            error_messages.append(f"\t{type} - {message}")
        return "\n".join(error_messages)

    def __eq__(self, other : 'ParsingErrors') -> bool:
        if isinstance(other, ParsingErrors):
            return self._errors == other._errors
        return False
    
    

    def __hash__(self) -> int:
        return hash(tuple(self._errors))

    def __iter__(self):
        return iter(self._errors)

    def __next__(self) -> LocationValidationResult:
        return next(self.__iter__())

    @classmethod
    def empty(cls):
        return cls()

