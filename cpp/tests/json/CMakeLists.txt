#==============================================================================
# dnv::vista::sdk::json - Tests
#==============================================================================

file(GLOB_RECURSE DNV_VISTA_SDK_TESTS_JSON_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(dnv-vista-sdk-tests-json ${DNV_VISTA_SDK_TESTS_JSON_SOURCES})

set_target_properties(dnv-vista-sdk-tests-json
    PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
        DEBUG_POSTFIX         "-d"
)

target_compile_definitions(dnv-vista-sdk-tests-json
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
)

target_include_directories(dnv-vista-sdk-tests-json
    PRIVATE
        ${DNV_VISTA_SDK_SOURCE_DIR}
)

target_link_libraries(dnv-vista-sdk-tests-json
    PRIVATE
        doctest::doctest_with_main
        dnv::vista::sdk
        dnv::vista::sdk::warnings
)

doctest_discover_tests(dnv-vista-sdk-tests-json)
