add_subdirectory(EngineBridge)
add_subdirectory(Log)
add_subdirectory(Core)
add_subdirectory(Module)
add_subdirectory(System)

file(GLOB_RECURSE MODULE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
generate_module_reflection("EngineCoreModule" OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)
add_library(EngineCore_Module STATIC ${MODULE_SOURCES} "${OUT_TARGET_FILES}")
target_include_directories(EngineCore_Module
    PUBLIC Public "${OUT_PUBLIC}"
    PRIVATE Private "${OUT_PRIVATE}")

target_link_libraries(EngineCore_Module PUBLIC 
    EngineCore_Module_Log 
    EngineCore_Module_System 
    EngineCore_Module_Module
    EngineCore_Module_EngineBridge
    EngineCore_Module_Core
)