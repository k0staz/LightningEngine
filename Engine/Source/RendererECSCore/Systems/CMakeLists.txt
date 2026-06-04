file(GLOB_RECURSE SYSTEMS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(RendererECSCore_Module_Systems STATIC ${SYSTEMS_SOURCES})
target_include_directories(RendererECSCore_Module_Systems
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(RendererECSCore_Module_Systems PUBLIC EngineCore_Module RendererECSCore_Module_Components Renderer_Module)