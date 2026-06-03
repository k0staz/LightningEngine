file(GLOB_RECURSE COMPONENTS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(CoreECS_Module_Components STATIC ${COMPONENTS_SOURCES})
target_include_directories(CoreECS_Module_Components
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(CoreECS_Module_Components PUBLIC EngineCore_Module)