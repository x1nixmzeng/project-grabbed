
FMT_DIR = path.join(SRC_EXTERNAL_DIR, 'fmt')

local LIB_NAME = "fmt"

project (LIB_NAME)
    uuid (os.uuid(LIB_NAME))
    kind "StaticLib"

    files {
        path.join(FMT_DIR, 'include', LIB_NAME, "format.h"),
        path.join(FMT_DIR, 'include', LIB_NAME, "os.h"),
        path.join(FMT_DIR, 'src', "*.cc"),
    }

    includedirs {
        path.join(FMT_DIR, 'include')
    }

    flags {
        "Cpp17",
    }

    defaultConfigurations()
