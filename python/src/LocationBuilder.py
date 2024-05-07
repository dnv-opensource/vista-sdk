from python.src.Locations import Location, LocationGroup


class LocationBuilder:
    def __init__(self, locations):
        self.vis_version = locations.vis_version
        self.reversed_groups = locations._reversed_groups
        self.number = None
        self.side = None
        self.vertical = None
        self.transverse = None
        self.longitudinal = None

    @staticmethod
    def create(locations):
        return LocationBuilder(locations)

    def with_location(self, value):
        builder = self.copy()
        n = None
        digits = ''.join(filter(str.isdigit, value))
        if digits:
            n = int(digits)

        for ch in value:
            if not ch.isdigit():
                builder = builder.with_value(ch)

        if n is not None:
            builder = builder.with_number(n)
        return builder

    def with_number(self, number):
        return self.with_value_internal(LocationGroup.NUMBER, number)

    def with_side(self, side):
        return self.with_value_internal(LocationGroup.SIDE, side)
    
    def with_vertical(self, vertical):
        return self.with_value_internal(LocationGroup.VERTICAL, vertical)

    def with_transverse(self, transverse):
        return self.with_value_internal(LocationGroup.TRANSVERSE, transverse)

    def with_longitudinal(self, longitudinal):
        return self.with_value_internal(LocationGroup.LONGITUDINAL, longitudinal)

    def with_value(self, value):
        return self.with_value_internal(LocationGroup.NUMBER, value)
    
    def with_value_char(self, value):
        if value not in self.reversed_groups:
            raise ValueError(f"The value {value} is an invalid Locations value")
        group = self.reversed_groups[value]
        return self.with_value_internal(group, value)

    def with_value_internal(self, group, value):
        if group == LocationGroup.NUMBER:
            if not isinstance(value, int):
                raise ValueError("Value should be a number")
            if value < 1:
                raise ValueError("Number must be greater than 0")
            builder = self.copy()
            builder.number = value
            return builder

        if not isinstance(value, str) or len(value) != 1:
            raise ValueError("Value should be a single character")
        
        if value not in self.reversed_groups or self.reversed_groups[value] != group:
            raise ValueError(f"The value {value} is an invalid {group.name} value")

        builder = self.copy()
        setattr(builder, group.name.lower(), value)
        return builder

    def without_value(self, group):
        builder = self.copy()
        setattr(builder, group.name.lower(), None)
        return builder

    def build(self):
        return Location(self.__str__())

    def __str__(self):
        parts = [self.side, self.vertical, self.transverse, self.longitudinal]
        parts = [str(p) for p in parts if p is not None]
        if self.number:
            parts.insert(0, str(self.number))
        return ''.join(sorted(parts))
    
    def without_number(self):
        return self.without_value(LocationGroup.NUMBER)

    def without_side(self):
        return self.without_value(LocationGroup.SIDE)

    def without_vertical(self):
        return self.without_value(LocationGroup.VERTICAL)

    def without_transverse(self):
        return self.without_value(LocationGroup.TRANSVERSE)

    def without_longitudinal(self):
        return self.without_value(LocationGroup.LONGITUDINAL)


    def copy(self):
        new_copy = LocationBuilder.__new__(LocationBuilder)
        new_copy.__init__(self)
        new_copy.number = self.number
        new_copy.side = self.side
        new_copy.vertical = self.vertical
        new_copy.transverse = self.transverse
        new_copy.longitudinal = self.longitudinal
        return new_copy
