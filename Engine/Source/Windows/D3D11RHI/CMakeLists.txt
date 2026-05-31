file(GLOB_RECURSE D3D11RHI_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(D3D11RHI STATIC ${D3D11RHI_SOURCES})
target_include_directories(D3D11RHI
    PUBLIC Public
    PRIVATE Private)

find_package(D3DCompiler REQUIRED)
if(D3DCompiler_FOUND)
    set_target_properties(D3D11RHI PROPERTIES
            INTERFACE_LINK_DEPENDS "${D3DCompiler_DLL_PATH}"
    )
endif()

target_link_libraries(D3D11RHI PUBLIC Log Core RHI Renderer)