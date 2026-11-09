#==============================================================================
# dnv::vista::sdk::core - Tests
#==============================================================================

file(GLOB_RECURSE DNV_VISTA_SDK_CORE_TESTS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(dnv-vista-sdk-tests-core ${DNV_VISTA_SDK_CORE_TESTS_SOURCES})

set_target_properties(dnv-vista-sdk-tests-core
    PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
        DEBUG_POSTFIX         "-d"
)

target_compile_definitions(dnv-vista-sdk-tests-core
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
)

target_include_directories(dnv-vista-sdk-tests-core
    PRIVATE
        ${DNV_VISTA_SDK_SOURCE_DIR}
)

target_link_libraries(dnv-vista-sdk-tests-core
    PRIVATE
        doctest::doctest_with_main
        dnv::vista::sdk
        dnv::vista::sdk::testdata
        dnv::vista::sdk::warnings
)

doctest_discover_tests(dnv-vista-sdk-tests-core)
