file(GLOB_RECURSE CLIENT_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Client STATIC ${CLIENT_SOURCES})
target_include_directories(Client
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Client PUBLIC Log Core Renderer EngineBridge ClientBridge)