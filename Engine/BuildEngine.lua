do
    local script = "../Engine/Tools/GenerateShaderRegistrationFiles.py"
    if os.isfile(script) then
        local result = os.execute("python " .. script)
    else
        print("Script not found: " .. script)
    end
end

project "EngineTemp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    publicIncludeDirs
    {
        "Source/Client/Public",
        "Source/Core/Public",
        "Source/CoreECS/Public",
        "Source/Engine/Public",
        "Source/Renderer/Public",
        "Source/RHI/Public",
        "Source/RHIRuntime/Public",
        "Source/System/Public",
        "Source/Windows/**/Public",

        "Generated/Public",
    }

    files { 
        "Source/Client/**.h", 
        "Source/Core/**.h",
        "Source/CoreECS/**.h",
        "Source/Engine/**.h",
        "Source/Renderer/**.h",
        "Source/RHI/**.h",
        "Source/RHIRuntime/**.h",
        "Source/System/**.h",
        "Source/Windows/**.h",

        "Source/Client/**.cpp", 
        "Source/Core/**.cpp",
        "Source/CoreECS/**.cpp",
        "Source/Engine/**.cpp",
        "Source/Renderer/**.cpp",
        "Source/RHI/**.cpp",
        "Source/RHIRuntime/**.cpp",
        "Source/System/**.cpp",
        "Source/Windows/**.cpp",

        "Generated/**.h",
        "Generated/**.cpp"
    }

    use_modules({"Log"})

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    register_project(project(), path.getdirectory(_SCRIPT))
 
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }
 

    prebuildcommands {
        "{ECHO} Running shader registry generator...",
        "python Tools/GenerateShaderRegistrationFiles.py"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"
 
    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"