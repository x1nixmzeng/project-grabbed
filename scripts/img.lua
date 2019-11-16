
local LIB_NAME = "img"

project (LIB_NAME)
    uuid (os.uuid(LIB_NAME))
    kind "StaticLib"

    links {
        "base",
        "dxt",
    }

    files {
        path.join(SRC_DIR, LIB_NAME, "**.h"),
        path.join(SRC_DIR, LIB_NAME, "**.cpp"),
        path.join(SRC_EXTERNAL_DIR, "DirectXTex", "DirectXTex", "DirectXTex.h"),
    }

    includedirs {
        path.join(SRC_DIR),
        path.join(SRC_EXTERNAL_DIR, "DirectXTex", "DirectXTex")
    }

    flags {
        "Cpp14",
    }

    defaultConfigurations()
