add_subdirectory(Renderer)
add_subdirectory(RHI)
add_subdirectory(Vulkan)

file(GLOB_RECURSE MODULE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
generate_module_reflection("RendererModule" OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)
add_library(Renderer_Module STATIC ${MODULE_SOURCES} "${OUT_TARGET_FILES}")
target_include_directories(Renderer_Module
    PUBLIC Public "${OUT_PUBLIC}"
    PRIVATE Private "${OUT_PRIVATE}")

target_link_libraries(Renderer_Module PUBLIC
        Renderer_Module_RHI
        Renderer_Module_Renderer
        PRIVATE Renderer_Module_Vulkan)