file(GLOB_RECURSE D3D11RHI_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_D3D11RHI STATIC ${D3D11RHI_SOURCES})
target_include_directories(Renderer_Module_D3D11RHI
    PUBLIC Public
    PRIVATE Private)

find_package(D3DCompiler REQUIRED)
if(D3DCompiler_FOUND)
    set_target_properties(Renderer_Module_D3D11RHI PROPERTIES
            INTERFACE_LINK_DEPENDS "${D3DCompiler_DLL_PATH}"
    )
endif()

target_link_libraries(Renderer_Module_D3D11RHI PUBLIC EngineCore_Module Renderer_Module_RHI Renderer_Module_Renderer)