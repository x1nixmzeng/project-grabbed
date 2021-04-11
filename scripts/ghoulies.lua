
local LIB_NAME = "ghoulies"

project (LIB_NAME)
    uuid (os.uuid(LIB_NAME))
    kind "StaticLib"

    links {
        "base",
    }

    files {
        path.join(SRC_DIR, LIB_NAME, "**.h"),
        path.join(SRC_DIR, LIB_NAME, "**.cpp"),
    }

    includedirs {
        path.join(SRC_DIR),
        path.join(FMT_DIR, 'include'),
    }

    flags {
        "Cpp17",
    }

    defaultConfigurations()
