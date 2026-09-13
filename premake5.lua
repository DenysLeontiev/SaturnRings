workspace "Instancing"
    architecture "x86"
    startproject "Instancing"

    configurations { "Debug", "Release" }
    platforms { "Win32" }

project "Instancing"
    location "Instancing"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    
    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}-%{cfg.architecture}")

    debugdir "%{wks.location}/Instancing"

    files {
        "Instancing/src/**.h",
        "Instancing/src/**.cpp",
        "Instancing/src/**.glsl",
        "Instancing/resources/**"
    }

    includedirs {
        "External/Assimp/include",
        "External/GLEW/include",
        "External/GLFW/include",
        "Instancing/src/vendor",
        "Instancing/src/vendor/imgui",
        "Instancing/src/vendor/stb"
    }

    libdirs {
        "External/Assimp/lib",
        "External/GLFW/lib-vc2022"
    }

    links {
        "assimp-vc143-mtd.lib",
        "opengl32.lib",
        "glew32s.lib",
        "glfw3.lib"
    }

    defines {
        "WIN32",
        "_CONSOLE",
        "GLEW_STATIC"
    }

    postbuildcommands {
        '{COPY} "%{wks.location}/External/Assimp/bin/*.dll" "%{cfg.targetdir}"',
        '{COPYDIR} "%{wks.location}/Instancing/resources" "%{cfg.targetdir}/resources"'
    }

    filter "platforms:Win32"
        architecture "x86"
        libdirs { "External/GLEW/lib/Release/Win32" }
    filter {}

    filter "configurations:Debug"
        defines { "_DEBUG" }
        symbols "On"
    filter {}

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter{}