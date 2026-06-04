file(GLOB_RECURSE RENDERER_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_Renderer STATIC ${RENDERER_SOURCES})
target_include_directories(Renderer_Module_Renderer
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Renderer_Module_Renderer PUBLIC EngineCore_Module Renderer_Module_RHI)