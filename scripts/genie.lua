newoption {
    trigger = "with-tools",
    description = "Enable building tools.",
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

BX_DIR              = path.join(SRC_EXTERNAL_DIR, "bx")
BGFX_DIR            = path.join(SRC_EXTERNAL_DIR, "bgfx")
BIMG_DIR            = path.join(SRC_EXTERNAL_DIR, "bimg")

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
    
    if _OPTIONS["with-bgfx"] then
        -- introduce bgfx toolchain early
        dofile (path.join(BX_DIR, "scripts/toolchain.lua"))

        if not toolchain(BUILD_DIR, BUILD_DIR) then
        return -- no action specified
        end    
    end
    
    
    startproject "app"

    location(ROOT_DIR)
    
    group "external"
        dofile "zlib.lua"
        dofile "dxt.lua"

    group "libs"
        dofile "base.lua"
        dofile "img.lua"
        dofile "ghoulies.lua"
        dofile "kameo.lua"
        dofile "conker_lr.lua"
        dofile "banjokazooie_nb.lua"
        dofile "pinata.lua"
        dofile "perfectdark.lua"
    
    if _OPTIONS["with-tools"] then
        group "tools"
            dofile "tools.lua"
            
        -- workaround for broken genie/premake logic
        fixOutputPaths()
    end
