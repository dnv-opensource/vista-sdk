#==============================================================================
# dnv-vista-sdk - Doxygen documentation
#==============================================================================

find_package(Doxygen QUIET COMPONENTS dot)

if(NOT DOXYGEN_FOUND)
    message(WARNING "Doxygen not found - DNV_VISTA_SDK_BUILD_DOCS disabled")
    set(DNV_VISTA_SDK_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    return()
endif()

find_program(GRAPHVIZ_DOT_EXECUTABLE NAMES dot)
if(DOXYGEN_DOT_FOUND OR GRAPHVIZ_DOT_EXECUTABLE)
    set(DNV_VISTA_SDK_HAVE_DOT "YES")
else()
    message(WARNING "Graphviz (dot) not found - dependency graphs disabled")
    set(DNV_VISTA_SDK_HAVE_DOT "NO")
endif()

if(DNV_VISTA_SDK_WARNINGS_AS_ERRORS)
    set(DNV_VISTA_SDK_DOXYGEN_WARN_AS_ERROR "YES")
else()
    set(DNV_VISTA_SDK_DOXYGEN_WARN_AS_ERROR "NO")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY
)

add_custom_target(dnv-vista-sdk-docs
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "html/index.html" "${CMAKE_CURRENT_BINARY_DIR}/docs/index.html"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating API documentation"
    VERBATIM
)
