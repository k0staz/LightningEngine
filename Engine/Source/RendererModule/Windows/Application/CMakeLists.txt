file(GLOB_RECURSE WINDOWS_APPLICATION_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_WindowsApplication STATIC ${WINDOWS_APPLICATION_SOURCES})
target_include_directories(Renderer_Module_WindowsApplication
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(Renderer_Module_WindowsApplication PUBLIC EngineCore_Module)