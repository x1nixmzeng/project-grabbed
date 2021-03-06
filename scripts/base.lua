
local LIB_NAME = "base"

project (LIB_NAME)
    uuid (os.uuid(LIB_NAME))
    kind "StaticLib"

    links {
        "zlib",
        "fmt",
    }

    files {
        path.join(SRC_DIR, LIB_NAME, "*.h"),
        path.join(SRC_DIR, LIB_NAME, "*.cpp"),
        path.join(SRC_DIR, LIB_NAME, "*.inl"),
        path.join(SRC_DIR, LIB_NAME, "*.natvis"),
    }

    includedirs {
        path.join(SRC_DIR),
        path.join(SRC_EXTERNAL_DIR),
        path.join(FMT_DIR, 'include'),
    }

    flags {
        "Cpp17",
    }

    defaultConfigurations()
