file(GLOB_RECURSE RHI_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_RHI STATIC ${RHI_SOURCES})
target_include_directories(Renderer_Module_RHI
    PUBLIC Public
    PRIVATE Private 3r)

if (NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    message(WARNING "Vulkan SDK not detected, if installed please set VULKAN_SDK env var or manually provide libraries")
else()
    message(STATUS "Found Vulkan SDK in $ENV{VULKAN_SDK}")
    
    target_include_directories(Renderer_Module_RHI PRIVATE $ENV{VULKAN_SDK}/include)
endif()

target_link_libraries(Renderer_Module_RHI PUBLIC EngineCore_Module PRIVATE slang)