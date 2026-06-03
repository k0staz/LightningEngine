file(GLOB_RECURSE COMPONENTS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(RendererECSCore_Module_Components STATIC ${COMPONENTS_SOURCES})
target_include_directories(RendererECSCore_Module_Components
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(RendererECSCore_Module_Components PUBLIC EngineCore_Module Renderer_Module)