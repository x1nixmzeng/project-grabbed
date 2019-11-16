
local ZLIB_DIR = path.join(SRC_EXTERNAL_DIR, "zlib")

project ("zlib")
	uuid (os.uuid("zlib"))
	kind "StaticLib"

	files {
		path.join(ZLIB_DIR, "crc32.h"),
		path.join(ZLIB_DIR, "deflate.h"),
		path.join(ZLIB_DIR, "gzguts.h"),
		path.join(ZLIB_DIR, "inffast.h"),
		path.join(ZLIB_DIR, "infast.h"),
		path.join(ZLIB_DIR, "inffixed.h"),
		path.join(ZLIB_DIR, "inflate.h"),
		path.join(ZLIB_DIR, "inftrees.h"),
		path.join(ZLIB_DIR, "trees.h"),
		path.join(ZLIB_DIR, "zconf.h"),
		path.join(ZLIB_DIR, "zlib.h"),
		path.join(ZLIB_DIR, "zutil.h"),

		path.join(ZLIB_DIR, "adler32.c"),
		path.join(ZLIB_DIR, "compress.c"),
		path.join(ZLIB_DIR, "crc32.c"),
		path.join(ZLIB_DIR, "deflate.c"),
		path.join(ZLIB_DIR, "gzclose.c"),
		path.join(ZLIB_DIR, "gzlib.c"),
		path.join(ZLIB_DIR, "gzread.c"),
		path.join(ZLIB_DIR, "gzwrite.c"),
		path.join(ZLIB_DIR, "infback.c"),
		path.join(ZLIB_DIR, "inffast.c"),
		path.join(ZLIB_DIR, "inflate.c"),
		path.join(ZLIB_DIR, "inftrees.c"),
		path.join(ZLIB_DIR, "trees.c"),
		path.join(ZLIB_DIR, "uncompr.c"),
		path.join(ZLIB_DIR, "zutil.c"),
	}

	includedirs {
		ZLIB_DIR,
	}
    
    defaultConfigurations()
