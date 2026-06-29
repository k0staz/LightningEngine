add_subdirectory(ImageDecoder)
add_subdirectory(GLTFImporter)

file(GLOB_RECURSE MODULE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
generate_module_reflection("EngineToolsModule" OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)
add_library(EngineTools_Module STATIC ${MODULE_SOURCES} "${OUT_TARGET_FILES}")
target_include_directories(EngineTools_Module
    PUBLIC Public "${OUT_PUBLIC}"
    PRIVATE Private "${OUT_PRIVATE}")

target_link_libraries(EngineTools_Module PUBLIC EngineTools_Module_ImageDecoder EngineTools_Module_GLTFImporter)