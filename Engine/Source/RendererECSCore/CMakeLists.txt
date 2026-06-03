add_subdirectory(Components)
add_subdirectory(Systems)


file(GLOB_RECURSE MODULE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
generate_module_reflection("RendererECSCoreModule" OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)
add_library(RendererECSCore_Module STATIC ${MODULE_SOURCES} "${OUT_TARGET_FILES}")
target_include_directories(RendererECSCore_Module
    PUBLIC Public "${OUT_PUBLIC}"
    PRIVATE Private "${OUT_PRIVATE}")

target_link_libraries(RendererECSCore_Module PUBLIC RendererECSCore_Module_Systems RendererECSCore_Module_Components)