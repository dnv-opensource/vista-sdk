#==============================================================================
# dnv-vista-sdk - CMake targets
#==============================================================================

#----------------------------------------------
# Warnings
#----------------------------------------------

add_library(dnv-vista-sdk-warnings INTERFACE)
add_library(dnv::vista::sdk::warnings ALIAS dnv-vista-sdk-warnings)

target_compile_options(dnv-vista-sdk-warnings
    INTERFACE
        $<$<AND:$<BOOL:${DNV_VISTA_SDK_WARNINGS}>,$<CXX_COMPILER_ID:GNU,Clang>>:-Wall -Wextra -Wpedantic>
        $<$<AND:$<BOOL:${DNV_VISTA_SDK_WARNINGS}>,$<CXX_COMPILER_ID:MSVC>>:/W4>
)
