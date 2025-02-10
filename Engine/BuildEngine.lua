project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    includedirs
    {
        "Source",
        "Source/Core/Public",
        "Source/Log/Public",
        "3rdParty/spdlog/include",
    }

    files { "Source/**.h", "Source/**.cpp" }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")
 
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }
 
    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"
 
    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"