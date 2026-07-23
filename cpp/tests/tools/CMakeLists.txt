#==============================================================================
# dnv::vista::sdk::tools - Tests
#==============================================================================

if(NOT DNV_VISTA_SDK_BUILD_TOOLS)
    return()
endif()

file(GLOB_RECURSE DNV_VISTA_SDK_TOOLS_TESTS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(dnv-vista-sdk-tests-tools
    ${DNV_VISTA_SDK_TOOLS_TESTS_SOURCES}
    ${DNV_VISTA_SDK_DIR}/tools/json-validator/SchemaValidator.cpp
)

set_target_properties(dnv-vista-sdk-tests-tools
    PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
        DEBUG_POSTFIX         "-d"
)

target_compile_definitions(dnv-vista-sdk-tests-tools
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
)

target_include_directories(dnv-vista-sdk-tests-tools
    PRIVATE
        ${DNV_VISTA_SDK_DIR}
        ${DNV_VISTA_SDK_SOURCE_DIR}
)

target_link_libraries(dnv-vista-sdk-tests-tools
    PRIVATE
        doctest::doctest_with_main
        dnv::vista::sdk
        dnv::vista::sdk::schemas
        dnv::vista::sdk::warnings
)

doctest_discover_tests(dnv-vista-sdk-tests-tools)
