do
    local script = "../../Tools/GenerateECSSystemRegistrationFiles.py"
    if os.isfile(script) then
        local result = os.execute("python " .. script)
    else
        print("Script not found: " .. script)
    end
end

project "CoreECS"
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
        "Generated/Public"
    }

    files { 
        "Public/**.h",
        "Private/**.cpp",
        "Generated/Public/**.h",
        "Generated/Private/**.cpp",
    }

    use_modules({"Log", "Core", "Renderer", "EngineBridge"})

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    register_project(project(), path.getdirectory(_SCRIPT))
 
     prebuildcommands {
        "{ECHO} Running shader registry generator...",
        "python ../../Tools/GenerateECSSystemRegistrationFiles.py"
    }

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