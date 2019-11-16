
local LIB_NAME = "pinata"

project (LIB_NAME)
    uuid (os.uuid(LIB_NAME))
    kind "StaticLib"

    links {
        "base",
    }

    files {
        path.join(SRC_DIR, LIB_NAME, "**.h"),
        path.join(SRC_DIR, LIB_NAME, "**.cpp"),
        path.join(SRC_DIR, LIB_NAME, "**.natvis"),
    }

    includedirs {
        path.join(SRC_DIR),
    }

    flags {
        "Cpp14",
    }

    defaultConfigurations()

    -- workaround for 'FASTLINK' error
    -- see https://stackoverflow.com/a/54084135

--[[
    location(path.join(PROJECT_DIR, _ACTION))

    configuration "Debug"
    
        targetdir ("../build/debug")
        
        -- this field just does not work
        --objdir ("objdebug")
                
        defines { "DEBUG", "_DEBUG" }

        buildoptions { "/ZI /Fd:../debug/pinata.pdb" }
        linkoptions { "/OUT:../debug/pinata.pdb" }

    configuration "Profile"

        targetdir ("../build/profile")

        defines { "NDEBUG" }
        flags { "Optimize" }

        buildoptions { "/ZI /Fd:../profile/pinata.pdb" }
        linkoptions { "/OUT:../profile/pinata.pdb" }

    configuration "Release"
        
        targetdir ("../build/release")
        
        -- this field just does not work
        --objdir ("objrelease")
        
        defines { "NDEBUG" }
        flags { "Optimize" }

]]--
