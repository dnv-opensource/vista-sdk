# from .GmodVersioningDto import GmodVersioningDto, GmodVersioningNodeChangesDto
# from .VisVersions import VisVersion

# class GmodVersioning:
#     def __init__(self, dto):
#         self._versionings_map = {}
#         self.initialize_versioning(dto)
    
#     def initialize_versioning(self, dto):
#         for versioning_dto in dto.items:
#             vis_version = VisVersion.parse(versioning_dto.key)
#             gmod_versioning_node = GmodVersioningNode(versioning_dto.value)
#             self._versionings_map[vis_version] = gmod_versioning_node