
local LIB_NAME = "perfectdark"

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
        "Cpp17",
    }

    defaultConfigurations()

