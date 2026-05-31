file(GLOB_RECURSE LOG_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Log STATIC ${LOG_SOURCES})
target_include_directories(Log
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Log PRIVATE spdlog_header_only)