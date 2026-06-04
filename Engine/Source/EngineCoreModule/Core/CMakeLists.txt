file(GLOB_RECURSE CORE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(EngineCore_Module_Core STATIC ${CORE_SOURCES})
target_include_directories(EngineCore_Module_Core
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(EngineCore_Module_Core PUBLIC 
    EngineCore_Module_Log 
    EngineCore_Module_System 
    Tracy::TracyClient
    )