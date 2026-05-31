file(GLOB_RECURSE CORE_ECS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(CoreECS STATIC ${CORE_ECS_SOURCES})
target_include_directories(CoreECS
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(CoreECS PUBLIC Log Core Renderer EngineBridge)