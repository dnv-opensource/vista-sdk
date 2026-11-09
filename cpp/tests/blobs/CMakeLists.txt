#==============================================================================
# dnv::vista::sdk::blobs - Tests
#==============================================================================

file(GLOB_RECURSE DNV_VISTA_SDK_BLOB_TESTS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(dnv-vista-sdk-tests-blobs ${DNV_VISTA_SDK_BLOB_TESTS_SOURCES})

set_target_properties(dnv-vista-sdk-tests-blobs
    PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
        DEBUG_POSTFIX         "-d"
)

target_compile_definitions(dnv-vista-sdk-tests-blobs
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
)

target_link_libraries(dnv-vista-sdk-tests-blobs
    PRIVATE
        doctest::doctest_with_main
        dnv::vista::sdk::resources
        dnv::vista::sdk::warnings
)

doctest_discover_tests(dnv-vista-sdk-tests-blobs)
