
local LIB_NAME = "kameo"

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
    }

    flags {
        "Cpp14",
    }

    defaultConfigurations()

