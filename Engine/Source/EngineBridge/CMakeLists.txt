file(GLOB_RECURSE ENGINE_BRIDGE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(EngineBridge STATIC ${ENGINE_BRIDGE_SOURCES})
target_include_directories(EngineBridge
    PUBLIC Public
    PRIVATE Private)