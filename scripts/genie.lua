newoption {
    trigger = "with-tools",
    description = "Enable building tools.",
}

newoption {
    trigger = "with-app",
    description = "Enable building imgui app.",
    allowed = {
        { "gl",   "OpenGL" },
        { "dx",   "DirectX 11" },
        { "vk",   "Vulkan" },
    }
}

newoption {
    trigger = "vs-toolset",
    value = "default",
    description = "Choose a VS toolset.",
    allowed = {
        { "default",   "Default" },
        { "clang",     "Clang" },
    }
}

newoption {
    trigger = "audio-impl",
    value = "stub",
    description = "Backend audio implementation",
    allowed = {
        { "stub",   "No audio playback" },
        { "fmod",   "FMOD (Windows only)" },
    }
}

newoption {
    trigger = "movie-impl",
    value = "stub",
    description = "Backend movie implementation",
    allowed = {
        { "stub",   "No movie playback" },
        { "bink",   "Bink (Windows x32 only)" },
    }
}

ROOT_DIR            = path.getabsolute("..")

PROJECT_DIR         = path.join(ROOT_DIR, "build")
BUILD_DIR           = path.join(ROOT_DIR, "build")

SRC_DIR             = path.join(ROOT_DIR, "src")
SRC_EXTERNAL_DIR    = path.join(SRC_DIR, "external")

function defaultConfigurations()

    location(path.join(PROJECT_DIR, _ACTION))

    configuration "Debug"
    
        targetdir ("../build/debug")
        
        -- this field just does not work
        --objdir ("objdebug")
                
        defines { "DEBUG", "_DEBUG" }
        flags { "Symbols", "FullSymbols" }

    configuration "Profile"

        targetdir ("../build/profile")

        defines { "NDEBUG" }
        flags { "Optimize", "Symbols" }

    configuration "Release"
        
        targetdir ("../build/release")
        
        -- this field just does not work
        --objdir ("objrelease")
        
        defines { "NDEBUG" }
        flags { "Optimize" }
        
end

-- Required for bgfx and example-common
function copyLib()
end


function fixOutputPaths()
    location(path.join(PROJECT_DIR, _ACTION))
    objdir(path.join(BUILD_DIR, _ACTION))
end


solution "grabbed"
    configurations {
        "Debug",
        "Profile",
        "Release",
    }

    platforms {
        "x32",
        "x64",
    }

    language "C++"
    
    startproject "app"

    location(ROOT_DIR)
    
    if "clang" == _OPTIONS["vs-toolset"] then
        premake.vstudio.toolset = ("ClangCL")
    end

    group "external"
        dofile "zlib.lua"
        dofile "dxt.lua"
        dofile "fmt.lua"

    group "libs"
        dofile "base.lua"
        dofile "img.lua"
        dofile "ghoulies.lua"
        dofile "kameo.lua"
        dofile "conker_lr.lua"
        dofile "banjokazooie_nb.lua"
        dofile "pinata.lua"
        dofile "perfectdark.lua"
        dofile "onevs.lua"
        dofile "xzp.lua"
    
    if _OPTIONS["with-tools"] then
        group "tools"
            dofile "tools.lua"
            
        -- workaround for broken genie/premake logic
        fixOutputPaths()
    end

    if _OPTIONS["with-app"] then
        group "app"
            dofile "app.lua"
    end

