import enum

class VisVersion(enum.Enum):
    v3_4a = "3-4a"
    v3_5a = "3-5a"
    v3_6a = "3-6a"
    v3_7a = "3-7a"


class VisVersionExtension:
    @staticmethod
    def to_version_string(version: VisVersion, builder=None) -> str:
        version_map = {
            VisVersion.v3_4a: "3-4a",
            VisVersion.v3_5a: "3-5a",
            VisVersion.v3_6a: "3-6a",
            VisVersion.v3_7a: "3-7a",
        }
        v = version_map.get(version, None)
        if v is None:
            raise ValueError(f'Invalid VisVersion enum value: {version}')
        if builder is not None:
            builder.append(v)
        return v

    @staticmethod
    def to_string(version, builder=None):
        return VisVersionExtension.to_version_string(version, builder)

    @staticmethod
    def is_valid(version):
        return isinstance(version, VisVersion)

class VisVersions:
    @staticmethod
    def all_versions():
        return [version for version in VisVersion if VisVersions.try_parse(version.value)]

    @staticmethod
    def try_parse(version_str) -> VisVersion:
        if version_str == "3-4a":
            return VisVersion.v3_4a
        if version_str == "3-5a":
            return VisVersion.v3_5a
        if version_str == "3-6a":
            return VisVersion.v3_6a
        if version_str == "3-7a":
            return VisVersion.v3_7a
        raise ValueError(f"Invalid VisVersion string : {version_str}")

    @staticmethod
    def parse(version_str: str) -> VisVersion:
        version = VisVersions.try_parse(version_str)
        if version is None:
            raise ValueError(f"Invalid VisVersion string: {version_str}")
        return version
