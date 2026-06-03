file(GLOB_RECURSE RHI_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_RHI STATIC ${RHI_SOURCES})
target_include_directories(Renderer_Module_RHI
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Renderer_Module_RHI PUBLIC EngineCore_Module)