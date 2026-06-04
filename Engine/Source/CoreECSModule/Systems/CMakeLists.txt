file(GLOB_RECURSE SYSTEMS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(CoreECS_Module_Systems STATIC ${SYSTEMS_SOURCES})
target_include_directories(CoreECS_Module_Systems
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(CoreECS_Module_Systems PUBLIC EngineCore_Module CoreECS_Module_Components)