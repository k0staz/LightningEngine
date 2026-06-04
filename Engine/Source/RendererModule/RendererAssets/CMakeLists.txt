file(GLOB_RECURSE RENDERER_ASSETS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_RendererAssets STATIC ${RENDERER_ASSETS_SOURCES})
target_include_directories(Renderer_Module_RendererAssets
        PUBLIC Public
        PRIVATE Private)

target_link_libraries(Renderer_Module_RendererAssets PUBLIC EngineCore_Module CoreECS_Module Renderer_Module_Renderer)