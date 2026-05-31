file(GLOB_RECURSE CLIENT_BRIDGE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(ClientBridge STATIC ${CLIENT_BRIDGE_SOURCES})
target_include_directories(ClientBridge
    PUBLIC Public
    PRIVATE Private)