file(GLOB_RECURSE SYSTEM_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(EngineCore_Module_System STATIC ${SYSTEM_SOURCES})
target_include_directories(EngineCore_Module_System
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(EngineCore_Module_System PUBLIC EngineCore_Module_Log)