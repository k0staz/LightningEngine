do
    local script = "../Engine/Tools/GenerateShaderRegistrationFiles.py"
    if os.isfile(script) then
        local result = os.execute("python " .. script)
    else
        print("Script not found: " .. script)
    end
end

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    includedirs
    {
        "Source/**/Public",
        "Generated/Public",
        "3rdParty/spdlog/include",
    }

    files { 
        "Source/**.h", 
        "Source/**.cpp",
        "Generated/**.h",
        "Generated/**.cpp"
    }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")
 
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