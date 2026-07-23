#==============================================================================
# JSON schema validator CLI
#==============================================================================

add_executable(dnv-vista-sdk-schema-validator
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SchemaValidator.cpp
)

set_target_properties(dnv-vista-sdk-schema-validator
    PROPERTIES
        OUTPUT_NAME            dnv-vista-sdk-json-validator-cli
        CXX_STANDARD           20
        CXX_STANDARD_REQUIRED  ON
        CXX_EXTENSIONS         OFF
        DEBUG_POSTFIX          "-d"
)

target_include_directories(dnv-vista-sdk-schema-validator
    PRIVATE
        ${DNV_VISTA_SDK_SOURCE_DIR}
)

target_link_libraries(dnv-vista-sdk-schema-validator
    PRIVATE
        dnv::vista::sdk
        dnv::vista::sdk::schemas
        dnv::vista::sdk::warnings
)
