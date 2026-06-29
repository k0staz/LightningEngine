file(GLOB_RECURSE TOOLS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h" "${ENGINE_EXTERNALS}/cgltf/*h")
add_library(EngineTools_Module_GLTFImporter STATIC ${TOOLS_SOURCES})
target_include_directories(EngineTools_Module_GLTFImporter
    PUBLIC Public
    PRIVATE Private "${ENGINE_EXTERNALS}/cgltf")

target_link_libraries(EngineTools_Module_GLTFImporter PUBLIC EngineCore_Module)