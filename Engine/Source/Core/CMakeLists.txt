file(GLOB_RECURSE CORE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Core STATIC ${CORE_SOURCES})
target_include_directories(Core
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Core PUBLIC Log System Tracy::TracyClient)