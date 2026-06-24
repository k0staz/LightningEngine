file(GLOB_RECURSE VULKAN_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(Renderer_Module_Vulkan STATIC ${VULKAN_SOURCES})
target_include_directories(Renderer_Module_Vulkan
        PUBLIC Public
        PRIVATE Private)

if (NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    message(WARNING "Vulkan SDK not detected, if installed please set VULKAN_SDK env var or manually provide libraries")
else()
    message(STATUS "Found Vulkan SDK in $ENV{VULKAN_SDK}")
    
    target_include_directories(Renderer_Module_Vulkan PRIVATE $ENV{VULKAN_SDK}/include)
    find_package(Vulkan REQUIRED)

    find_library(SDL_LIBRARY NAMES SDL3 HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
    target_link_libraries(Renderer_Module_RHI PRIVATE ${SDL_LIBRARY})
endif()

target_link_libraries(Renderer_Module_Vulkan PUBLIC EngineCore_Module Renderer_Module_RHI Renderer_Module_Renderer)