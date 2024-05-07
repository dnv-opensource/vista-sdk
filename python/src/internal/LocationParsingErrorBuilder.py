class LocationParsingErrorBuilder:
    def __init__(self):
        self._errors = []

    def add_error(self, name, message):
        self._errors.append((name, message))
        return self

    @property
    def has_error(self):
        return len(self._errors) > 0

    @staticmethod
    def create():
        return LocationParsingErrorBuilder()

    def build(self):
        return ParsingErrors(self._errors) if self._errors else ParsingErrors.empty

class ParsingErrors:
    empty = []

    def __init__(self, errors):
        self.errors = [(error[0], error[1]) for error in errors]

    def __repr__(self):
        return f"ParsingErrors({self.errors})"

