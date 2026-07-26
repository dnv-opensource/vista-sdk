#==============================================================================
# dnv::vista::sdk - Emscripten/WebGL sample
#==============================================================================

#----------------------------------------------
# Toolchain guard
#----------------------------------------------

if(NOT EMSCRIPTEN)
    message(STATUS "[emscripten-sample] Not an Emscripten build - skipping")
    return()
endif()

#----------------------------------------------
# Dependencies via FetchContent
#----------------------------------------------

include(FetchContent)

# ImGui
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9b
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/pongasoft/emscripten-glfw.git
    GIT_TAG        v3.5.1.20260809
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(imgui)
FetchContent_Populate(glfw)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glfw_SOURCE_DIR}/include
    ${glfw_SOURCE_DIR}/external
)

target_compile_definitions(imgui PUBLIC
    EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3
)

#----------------------------------------------
# Sample executable
#----------------------------------------------

add_executable(dnv-vista-sdk-sample-emscripten
    main.cpp
)

set_target_properties(dnv-vista-sdk-sample-emscripten
    PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
        OUTPUT_NAME           "gmod-explorer"
        SUFFIX                ".html"
)

target_include_directories(dnv-vista-sdk-sample-emscripten
    PRIVATE
        ${glfw_SOURCE_DIR}/include
        ${glfw_SOURCE_DIR}/external
)

target_compile_definitions(dnv-vista-sdk-sample-emscripten
    PRIVATE
        EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3
)

target_link_libraries(dnv-vista-sdk-sample-emscripten
    PRIVATE
        dnv::vista::sdk
        imgui
)

#----------------------------------------------
# Emscripten link flags
#----------------------------------------------

# SSE2 (__m128i) is not enabled by default in Emscripten; the SDK JSON fast
# paths are gated on __SSE2__ so they fall back to scalar automatically.
# Use -msimd128 only if you want WASM SIMD (requires a compatible browser).

target_link_options(dnv-vista-sdk-sample-emscripten
    PRIVATE
        --use-port=contrib.glfw3
        -sUSE_WEBGL2=1
        -sFULL_ES2=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=67108864
        "-sEXPORTED_RUNTIME_METHODS=['ccall']"
        -sMINIFY_HTML=0
        --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
)
