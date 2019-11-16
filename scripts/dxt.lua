
local DXT_DIR = path.join(SRC_EXTERNAL_DIR, "DirectXTex", "DirectXTex")

project ("dxt")
	uuid (os.uuid("dxt"))
	kind "StaticLib"

	files {
		path.join(DXT_DIR, "BC.h"),
		path.join(DXT_DIR, "BCDirectCompute.h"),
		path.join(DXT_DIR, "d3dx12.h"),
		path.join(DXT_DIR, "DDS.h"),
		path.join(DXT_DIR, "DirectXTex.h"),
		path.join(DXT_DIR, "DirectXTexP.h"),
		path.join(DXT_DIR, "filters.h"),
		path.join(DXT_DIR, "scoped.h"),
		path.join(DXT_DIR, "BC.cpp"),
		path.join(DXT_DIR, "BC4BC5.cpp"),
		path.join(DXT_DIR, "BC6HBC7.cpp"),
		path.join(DXT_DIR, "BCDirectCompute.cpp"),
		path.join(DXT_DIR, "DirectXTexCompress.cpp"),
		path.join(DXT_DIR, "DirectXTexCompressGPU.cpp"),
		path.join(DXT_DIR, "DirectXTexConvert.cpp"),
		path.join(DXT_DIR, "DirectXTexD3D11.cpp"),
		path.join(DXT_DIR, "DirectXTexD3D12.cpp"),
		path.join(DXT_DIR, "DirectXTexDDS.cpp"),
		path.join(DXT_DIR, "DirectXTexFlipRotate.cpp"),
		path.join(DXT_DIR, "DirectXTexHDR.cpp"),
		path.join(DXT_DIR, "DirectXTexImage.cpp"),
		path.join(DXT_DIR, "DirectXTexMipmaps.cpp"),
		path.join(DXT_DIR, "DirectXTexMisc.cpp"),
		path.join(DXT_DIR, "DirectXTexNormalMaps.cpp"),
		path.join(DXT_DIR, "DirectXTexPMAlpha.cpp"),
		path.join(DXT_DIR, "DirectXTexResize.cpp"),
		path.join(DXT_DIR, "DirectXTexTGA.cpp"),
		path.join(DXT_DIR, "DirectXTexUtil.cpp"),
		path.join(DXT_DIR, "DirectXTexWIC.cpp"),
	}

	includedirs {
		DXT_DIR,
	}

        defines { "_UNICODE", "UNICODE", "_WIN32_WINNT=0x0A00" }

	-- hack to avoid defaulting to 8.1
	-- see https://github.com/bkaradzic/GENie/issues/360

	local action = premake.action.current()
	action.vstudio.windowsTargetPlatformVersion = "10.0.16299.0"
    
    defaultConfigurations()

