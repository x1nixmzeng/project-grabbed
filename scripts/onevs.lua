
local LIB_NAME = "onevs"

project (LIB_NAME)
    uuid (os.uuid(LIB_NAME))
    kind "StaticLib"

    links {
        "base",
        "img",
    }

    files {
        path.join(SRC_DIR, LIB_NAME, "**.h"),
        path.join(SRC_DIR, LIB_NAME, "**.cpp"),
    }

    includedirs {
        path.join(SRC_DIR),
        path.join(SRC_DIR, "img", "img.h"),
    }

    flags {
        "Cpp17",
    }

    defaultConfigurations()
