file(GLOB_RECURSE MODULE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(EngineCore_Module_Module STATIC ${MODULE_SOURCES})
target_include_directories(EngineCore_Module_Module
        PUBLIC Public
        PRIVATE Private)

target_link_libraries(EngineCore_Module_Module PUBLIC EngineCore_Module_Core EngineCore_Module_Log)