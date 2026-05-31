file(GLOB_RECURSE SYSTEM_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(System STATIC ${SYSTEM_SOURCES})
target_include_directories(System
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(System PUBLIC Log)