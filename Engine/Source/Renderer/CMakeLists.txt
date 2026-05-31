file(GLOB_RECURSE RENDERER_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer STATIC ${RENDERER_SOURCES})
target_include_directories(Renderer
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Renderer PUBLIC Log Core RHI ClientBridge)