
local LIB_NAME = "banjokazooie_nb"

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
        path.join(FMT_DIR, 'include'),
    }

    flags {
        "Cpp17",
    }

    defaultConfigurations()
