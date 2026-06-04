file(GLOB_RECURSE LOG_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(EngineCore_Module_Log STATIC ${LOG_SOURCES})
target_include_directories(EngineCore_Module_Log
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(EngineCore_Module_Log PRIVATE spdlog_header_only)