file(GLOB_RECURSE ENGINE_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")

add_library(Engine STATIC ${ENGINE_SOURCES})

target_include_directories(Engine
    PUBLIC Public
    PRIVATE Private
)

target_link_libraries(Engine 
    PUBLIC 
        EngineBridge 
        Log 
        Core 
        CoreECS
        System
        RHI
        D3D11RHI
        Renderer
        Client
        WindowsApplication
        Tools
        AutoRegistration
)