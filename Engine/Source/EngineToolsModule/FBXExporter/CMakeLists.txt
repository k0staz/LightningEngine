file(GLOB_RECURSE TOOLS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h" "../../3rdParty/OpenFBX/src/*")
add_library(EngineTools_Module_FBXExporter STATIC ${TOOLS_SOURCES})
target_include_directories(EngineTools_Module_FBXExporter
    PUBLIC Public
    PRIVATE Private "../../../3rdParty/OpenFBX/src")

target_link_libraries(EngineTools_Module_FBXExporter PUBLIC Renderer_Module EngineCore_Module)