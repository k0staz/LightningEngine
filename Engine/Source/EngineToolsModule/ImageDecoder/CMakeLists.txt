file(GLOB_RECURSE TOOLS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h" "${ENGINE_EXTERNALS}/stb/*h")
add_library(EngineTools_Module_ImageDecoder STATIC ${TOOLS_SOURCES})
target_include_directories(EngineTools_Module_ImageDecoder
    PUBLIC Public
    PRIVATE Private "${ENGINE_EXTERNALS}/stb")

target_link_libraries(EngineTools_Module_ImageDecoder PUBLIC EngineCore_Module)