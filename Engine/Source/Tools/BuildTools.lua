project "Tools"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    publicIncludeDirs
    {
        "Public"
    }

    privateIncludeDirs
    {
        "../../3rdParty/OpenFBX/src",
    }

    files { 
        "Public/**.h",
        "Private/**.cpp",
        "../../3rdParty/OpenFBX/src/**.cpp",
        "../../3rdParty/OpenFBX/src/**.c"
    }

    use_modules({"Core", "CoreECS"})

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