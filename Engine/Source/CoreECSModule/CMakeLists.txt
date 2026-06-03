add_subdirectory(Components)
add_subdirectory(Systems)


file(GLOB_RECURSE MODULE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
generate_module_reflection("CoreECSModule" OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)
add_library(CoreECS_Module STATIC ${MODULE_SOURCES} "${OUT_TARGET_FILES}")
target_include_directories(CoreECS_Module
    PUBLIC Public "${OUT_PUBLIC}"
    PRIVATE Private "${OUT_PRIVATE}")

target_link_libraries(CoreECS_Module PUBLIC CoreECS_Module_Systems CoreECS_Module_Components)