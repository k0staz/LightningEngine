do
    local scripts = {
        "../../Tools/GenerateShaderRegistrationFiles.py",
        "../../Tools/GenerateECSSystemRegistrationFiles.py",
        "../../Tools/GenerateAssetTypeRegistrationFiles.py"
    }

    for i, value in ipairs(scripts) do
        if os.isfile(value) then
            local result = os.execute("python " .. value)
        else
            print("Script not found: " .. value)
        end
    end
end

project "AutoRegistration"
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

    use_modules({"Log", "Core", "RHI", "CoreECS", "Renderer"})

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    register_project(project(), path.getdirectory(_SCRIPT))

    prebuildcommands {
        "{ECHO} Running shader registry generator...",
        "python ../../Tools/GenerateShaderRegistrationFiles.py %{table.concat(cfg.links, ',')}",
        "{ECHO} Running ECS System registry generator...",
        "python ../../Tools/GenerateECSSystemRegistrationFiles.py %{table.concat(cfg.links, ',')}",
        "{ECHO} Running Asset Type registry generator...",
        "python ../../Tools/GenerateAssetTypeRegistrationFiles.py %{table.concat(cfg.links, ',')}"
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