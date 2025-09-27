project "Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    publicIncludeDirs
    {
        "Public"
    }

    publicIncludeDirs
    {
        "../../3rdParty/tracy/public/",
    }

    files { 
        "Public/**.h",
        "Private/**.cpp",
        "../../3rdParty/tracy/public/TracyClient.cpp",
        "../../3rdParty/tracy/public/**.hpp",
    }

    use_modules({"Log", "System"})

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    register_project(project(), path.getdirectory(_SCRIPT))
 
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