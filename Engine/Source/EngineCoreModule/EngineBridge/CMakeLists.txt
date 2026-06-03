file(GLOB_RECURSE ENGINE_BRIDGE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(EngineCore_Module_EngineBridge STATIC ${ENGINE_BRIDGE_SOURCES})
target_include_directories(EngineCore_Module_EngineBridge
    PUBLIC Public
    PRIVATE Private)