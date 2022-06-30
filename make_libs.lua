local action = _ACTION or ""
local todir = "build/" .. action

solution "gslib"
	configurations {
		"Debug",
		"Release"
	}
	location (todir)
	
	defines { "GS_LIB", "ARIEL_LIB" }
	
	-- no rtti
	rtti "Off"
	
	-- debug configs
	filter "configurations:Debug"
		defines { "DEBUG", "_DEBUG" }
		symbols "On"
		optimize "Debug"

	-- release configs
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "Full"
		
	-- windows specific
	filter "system:Windows"
		defines { "WIN32", "_WINDOWS", "_UNICODE" }

project "zlib"
	language "C"
	kind "StaticLib"
	entrypoint ""
	defines {
		"NO_FSEEKO",
		"_CRT_SECURE_NO_DEPRECATE",
		"_CRT_NONSTDC_NO_DEPRECATE"
	}
	includedirs {
		todir,
		"ext/zlib"
	}
	files {
		"ext/zlib/adler32.c",
		"ext/zlib/compress.c",
		"ext/zlib/crc32.c",
		"ext/zlib/deflate.c",
		"ext/zlib/gzclose.c",
		"ext/zlib/gzlib.c",
		"ext/zlib/gzread.c",
		"ext/zlib/gzwrite.c",
		"ext/zlib/infback.c",
		"ext/zlib/inffast.c",
		"ext/zlib/inflate.c",
		"ext/zlib/inftrees.c",
		"ext/zlib/trees.c",
		"ext/zlib/uncompr.c",
		"ext/zlib/zutil.c"
	}
	
project "freetype"
	language "C"
	kind "StaticLib"
	entrypoint ""
	defines { "_LIB", "_CRT_SECURE_NO_WARNINGS", "FT2_BUILD_LIBRARY" }
	includedirs {
		"ext/freetype/include"
	}
	files {
		"ext/freetype/src/autofit/autofit.c",
		"ext/freetype/src/base/ftbase.c",
		"ext/freetype/src/base/ftbbox.c",
		"ext/freetype/src/base/ftbdf.c",
		"ext/freetype/src/base/ftbitmap.c",
		"ext/freetype/src/base/ftcid.c",
		"ext/freetype/src/base/ftfstype.c",
		"ext/freetype/src/base/ftgasp.c",
		"ext/freetype/src/base/ftglyph.c",
		"ext/freetype/src/base/ftgxval.c",
		"ext/freetype/src/base/ftinit.c",
		"ext/freetype/src/base/ftmm.c",
		"ext/freetype/src/base/ftotval.c",
		"ext/freetype/src/base/ftpatent.c",
		"ext/freetype/src/base/ftpfr.c",
		"ext/freetype/src/base/ftstroke.c",
		"ext/freetype/src/base/ftsynth.c",
		"ext/freetype/src/base/ftsystem.c",
		"ext/freetype/src/base/fttype1.c",
		"ext/freetype/src/base/ftwinfnt.c",
		"ext/freetype/src/bdf/bdf.c",
		"ext/freetype/src/cache/ftcache.c",
		"ext/freetype/src/cff/cff.c",
		"ext/freetype/src/cid/type1cid.c",
		"ext/freetype/src/gzip/ftgzip.c",
		"ext/freetype/src/lzw/ftlzw.c",
		"ext/freetype/src/pcf/pcf.c",
		"ext/freetype/src/pfr/pfr.c",
		"ext/freetype/src/psaux/psaux.c",
		"ext/freetype/src/pshinter/pshinter.c",
		"ext/freetype/src/psnames/psmodule.c",
		"ext/freetype/src/raster/raster.c",
		"ext/freetype/src/sfnt/sfnt.c",
		"ext/freetype/src/smooth/smooth.c",
		"ext/freetype/src/truetype/truetype.c",
		"ext/freetype/src/type1/type1.c",
		"ext/freetype/src/type42/type42.c",
		"ext/freetype/src/winfonts/winfnt.c",
		"ext/freetype/src/ftdebug.c",
		"ext/freetype/src/base/ftver.rc"
	}
	filter "configurations:Debug"
		defines { "FT_DEBUG_LEVEL_ERROR", "FT_DEBUG_LEVEL_TRACE" }
	
project "libpng"
	language "C"
	kind "StaticLib"
	entrypoint ""
	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}
	includedirs {
		todir,
		"ext/zlib",
		"ext/libpng"
	}
	files {
		"ext/libpng/png.c",
		"ext/libpng/pngerror.c",
		"ext/libpng/pngget.c",
		"ext/libpng/pngmem.c",
		"ext/libpng/pngpread.c",
		"ext/libpng/pngread.c",
		"ext/libpng/pngrio.c",
		"ext/libpng/pngrtran.c",
		"ext/libpng/pngrutil.c",
		"ext/libpng/pngset.c",
		"ext/libpng/pngtrans.c",
		"ext/libpng/pngwio.c",
		"ext/libpng/pngwrite.c",
		"ext/libpng/pngwtran.c",
		"ext/libpng/pngwutil.c",
		"ext/libpng/intel/intel_init.c",
		"ext/libpng/intel/filter_sse2_intrinsics.c"
	}
	
project "libjpeg"
	language "C"
	kind "StaticLib"
	entrypoint ""
	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}
	includedirs {
		todir,
		"ext/libjpeg"
	}
	files {
		"ext/libjpeg/jcapimin.c",
		"ext/libjpeg/jcapistd.c",
		"ext/libjpeg/jctrans.c",
		"ext/libjpeg/jcparam.c",
		"ext/libjpeg/jdatadst.c",
		"ext/libjpeg/jcinit.c",
		"ext/libjpeg/jcmaster.c",
		"ext/libjpeg/jcmarker.c",
		"ext/libjpeg/jcmainct.c",
		"ext/libjpeg/jcprepct.c",
		"ext/libjpeg/jccoefct.c",
		"ext/libjpeg/jccolor.c",
		"ext/libjpeg/jcsample.c",
		"ext/libjpeg/jchuff.c",
		"ext/libjpeg/jcdctmgr.c",
		"ext/libjpeg/jfdctfst.c",
		"ext/libjpeg/jfdctflt.c",
		"ext/libjpeg/jfdctint.c",
		"ext/libjpeg/jdapimin.c",
		"ext/libjpeg/jdapistd.c",
		"ext/libjpeg/jdtrans.c",
		"ext/libjpeg/jdatasrc.c",
		"ext/libjpeg/jdmaster.c",
		"ext/libjpeg/jdinput.c",
		"ext/libjpeg/jdmarker.c",
		"ext/libjpeg/jdhuff.c",
		"ext/libjpeg/jdmainct.c",
		"ext/libjpeg/jdcoefct.c",
		"ext/libjpeg/jdpostct.c",
		"ext/libjpeg/jddctmgr.c",
		"ext/libjpeg/jidctfst.c",
		"ext/libjpeg/jidctflt.c",
		"ext/libjpeg/jidctint.c",
		"ext/libjpeg/jdsample.c",
		"ext/libjpeg/jdcolor.c",
		"ext/libjpeg/jquant1.c",
		"ext/libjpeg/jquant2.c",
		"ext/libjpeg/jdmerge.c",
		"ext/libjpeg/jcomapi.c",
		"ext/libjpeg/jutils.c",
		"ext/libjpeg/jerror.c",
		"ext/libjpeg/jmemmgr.c",
		"ext/libjpeg/jmemnobs.c",
		"ext/libjpeg/jaricom.c",
		"ext/libjpeg/jcarith.c",
		"ext/libjpeg/jdarith.c"
	}

project "gslib"
	language "C++"
	kind "StaticLib"
	entrypoint ""
	dependson {
		"zlib"
	}
	includedirs {
		todir,
		"include",
		"ext"
	}
	files {
		"include/gslib/avl.h",
		"include/gslib/base64.h",
		"include/gslib/basetype.h",
		"include/gslib/bintree.h",
		"include/gslib/config.h",
		"include/gslib/dir.h",
		"include/gslib/dvt.h",
		"include/gslib/error.h",
		"include/gslib/file.h",
		"include/gslib/gamma.h",
		"include/gslib/json.h",
		"include/gslib/library.h",
		"include/gslib/linequ.h",
		"include/gslib/math.h",
		"include/gslib/math.inl",
		"include/gslib/md5.h",
		"include/gslib/mtrand.h",
		"include/gslib/pool.h",
		"include/gslib/rbtree.h",
		"include/gslib/res.h",
		"include/gslib/rtree.h",
		"include/gslib/sha1.h",
		"include/gslib/std.h",
		"include/gslib/string.h",
		"include/gslib/thdpool.h",
		"include/gslib/tree.h",
		"include/gslib/treeop.h",
		"include/gslib/type.h",
		"include/gslib/utility.h",
		"include/gslib/uuid.h",
		"include/gslib/vdir.h",
		"include/gslib/xml.h",
		"include/gslib/zip.h",
		"src/gslib/base64.cpp",
		"src/gslib/dvt.cpp",
		"src/gslib/error.cpp",
		"src/gslib/gamma.cpp",
		"src/gslib/json.cpp",
		"src/gslib/library.cpp",
		"src/gslib/math.cpp",
		"src/gslib/md5.cpp",
		"src/gslib/mtrand.cpp",
		"src/gslib/res.cpp",
		"src/gslib/sha1.cpp",
		"src/gslib/string.cpp",
		"src/gslib/type.cpp",
		"src/gslib/utility.cpp",
		"src/gslib/uuid.cpp",
		"src/gslib/vdir.cpp",
		"src/gslib/xml.cpp",
		"src/gslib/zip.cpp",
		"src/gslib/zipimp.c"
	}

project "ariel"
	language "C++"
	kind "StaticLib"
	entrypoint ""
	includedirs {
		todir,
		"include",
		"src",
		"ext"
	}
	libdirs {
		"$(OutDir)"
	}
	prebuildcommands {
		'fxc /T vs_4_0 /E "rose_vsf_cr" /Fd /Zi /Fh "rose_vsf_cr.h" "../../src/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_psf_cr" /Fd /Zi /Fh "rose_psf_cr.h" "../../src/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vsf_klm_cr" /Fd /Zi /Fh "rose_vsf_klm_cr.h" "../../src/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_psf_klm_cr" /Fd /Zi /Fh "rose_psf_klm_cr.h" "../../src/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vss_coef_cr" /Fd /Zi /Fh "rose_vss_coef_cr.h" "../../src/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_pss_coef_cr" /Fd /Zi /Fh "rose_pss_coef_cr.h" "../../src/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vsf_klm_tex" /Fd /Zi /Fh "rose_vsf_klm_tex.h" "../../src/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_psf_klm_tex" /Fd /Zi /Fh "rose_psf_klm_tex.h" "../../src/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vss_coef_tex" /Fd /Zi /Fh "rose_vss_coef_tex.h" "../../src/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_pss_coef_tex" /Fd /Zi /Fh "rose_pss_coef_tex.h" "../../src/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "ariel_smaa_edge_detection_vs" /Fd /Zi /Fh "ariel_smaa_edge_detection_vs.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T ps_4_1 /E "ariel_smaa_luma_edge_detection_ps" /Fd /Zi /Fh "ariel_smaa_luma_edge_detection_ps.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T ps_4_1 /E "ariel_smaa_color_edge_detection_ps" /Fd /Zi /Fh "ariel_smaa_color_edge_detection_ps.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T vs_4_0 /E "ariel_smaa_blending_weight_calculation_vs" /Fd /Zi /Fh "ariel_smaa_blending_weight_calculation_vs.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T ps_4_1 /E "ariel_smaa_blending_weight_calculation_ps" /Fd /Zi /Fh "ariel_smaa_blending_weight_calculation_ps.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T vs_4_0 /E "ariel_smaa_neighborhood_blending_vs" /Fd /Zi /Fh "ariel_smaa_neighborhood_blending_vs.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T ps_4_1 /E "ariel_smaa_neighborhood_blending_ps" /Fd /Zi /Fh "ariel_smaa_neighborhood_blending_ps.h" "../../src/ariel/smaa.hlsl"',
		'fxc /T cs_5_0 /E "ariel_transpose_image_cs" /Fd /Zi /Fh "ariel_transpose_image_cs.h" "../../src/ariel/textureop.hlsl"',
		'fxc /T cs_5_0 /E "ariel_initialize_image_cs" /Fd /Zi /Fh "ariel_initialize_image_cs.h" "../../src/ariel/textureop.hlsl"',
		'fxc /T cs_5_0 /E "ariel_set_brightness_cs" /Fd /Zi /Fh "ariel_set_brightness_cs.h" "../../src/ariel/textureop.hlsl"',
		'fxc /T cs_5_0 /E "ariel_set_gray_cs" /Fd /Zi /Fh "ariel_set_gray_cs.h" "../../src/ariel/textureop.hlsl"',
		'fxc /T cs_5_0 /E "ariel_set_fade_cs" /Fd /Zi /Fh "ariel_set_fade_cs.h" "../../src/ariel/textureop.hlsl"',
		'fxc /T cs_5_0 /E "ariel_set_inverse_cs" /Fd /Zi /Fh "ariel_set_inverse_cs.h" "../../src/ariel/textureop.hlsl"',
		'fxc /T cs_5_0 /E "ariel_conv_from_premul_cs" /Fd /Zi /Fh "ariel_conv_from_premul_cs.h" "../../src/ariel/textureop.hlsl"'
	}
	files {
		"include/ariel/application.h",
		"include/ariel/batch.h",
		"include/ariel/classicstyle.h",
		"include/ariel/clip.h",
		"include/ariel/config.h",
		"include/ariel/delaunay.h",
		"include/ariel/dirty.h",
		"include/ariel/framesys.h",
		"include/ariel/fsyswin32.h",
		"include/ariel/fsysdwrite.h",
		"include/ariel/image.h",
		"include/ariel/imageop.h",
		"include/ariel/imageio.h",
		"include/ariel/loopblinn.h",
		"include/ariel/mesh.h",
		"include/ariel/painter.h",
		"include/ariel/painterpath.h",
		"include/ariel/painterport.h",
		"include/ariel/rectpack.h",
		"include/ariel/render.h",
		"include/ariel/rendersys.h",
		"include/ariel/rendersysd3d11.h",
		"include/ariel/resourcemgr.h",
		"include/ariel/rose.h",
		"include/ariel/sskeleton.h",
		"include/ariel/style.h",
		"include/ariel/scene.h",
		"include/ariel/scenemgr.h",
		"include/ariel/smaa.h",
		"include/ariel/sysop.h",
		"include/ariel/temporal.h",
		"include/ariel/texbatch.h",
		"include/ariel/textureop.h",
		"include/ariel/type.h",
		"include/ariel/widget.h",
		"include/ariel/classicstyle/slider_service.h",
		"include/ariel/classicstyle/scrollarea_service.h",
		"include/ariel/io/utilities.h",
		"include/ariel/io/mesh.h",
		"src/ariel/application.cpp",
		"src/ariel/batch.cpp",
		"src/ariel/classicstyle.cpp",
		"src/ariel/clip.cpp",
		"src/ariel/delaunay.cpp",
		"src/ariel/dirty.cpp",
		"src/ariel/framesys.cpp",
		"src/ariel/framesyswin32.cpp",
		"src/ariel/fsyswin32.cpp",
		"src/ariel/fsysdwrite.cpp",
		"src/ariel/image.cpp",
		"src/ariel/imageop.cpp",
		"src/ariel/imageio.cpp",
		"src/ariel/loopblinn.cpp",
		"src/ariel/mesh.cpp",
		"src/ariel/painter.cpp",
		"src/ariel/painterpath.cpp",
		"src/ariel/painterport.cpp",
		"src/ariel/rectpack.cpp",
		"src/ariel/render.cpp",
		"src/ariel/rendersys.cpp",
		"src/ariel/rendersysd3d11.cpp",
		"src/ariel/resourcemgr.cpp",
		"src/ariel/rose.cpp",
		"src/ariel/sskeleton.cpp",
		"src/ariel/scene.cpp",
		"src/ariel/scenemgr.cpp",
		"src/ariel/style.cpp",
		"src/ariel/smaa.cpp",
		"src/ariel/temporal.cpp",
		"src/ariel/texbatch.cpp",
		"src/ariel/textureop.cpp",
		--"src/ariel/textureop.hlsl",
		"src/ariel/widget.cpp",
		--"src/ariel/rose.hlsl",
		--"src/ariel/smaa.hlsl",
		"src/ariel/io/utilities.cpp",
		"src/ariel/io/mesh.cpp"
	}
	
project "rathen"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"code/rathen/basis.cpp",
		"code/rathen/basis.h",
		"code/rathen/buildin.cpp",
		"code/rathen/buildin.h",
		"code/rathen/compiler.cpp",
		"code/rathen/compiler.h",
		"code/rathen/config.h",
		"code/rathen/lexical.h",
		"code/rathen/library.cpp",
		"code/rathen/library.h",
		"code/rathen/loader.cpp",
		"code/rathen/loader.h",
		"code/rathen/pack.cpp",
		"code/rathen/pack.h",
		"code/rathen/parser.cpp",
		"code/rathen/parser.h",
		"code/rathen/proc.cpp",
		"code/rathen/proc.h",
		"code/rathen/rasm.cpp",
		"code/rathen/rasm.h",
		"code/rathen/tuple.cpp",
		"code/rathen/tuple.h",
		"code/gslib/error.h",
		"code/gslib/error.cpp",
		"code/gslib/string.cpp",
		"code/gslib/string.h",
		"code/gslib/std.h",
		"code/gslib/type.h",
		"code/gslib/tree.h",
		"code/gslib/pool.h",
		"proj/rathen/main.cpp"
	}
	
-- following are tools
project "mres"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	includedirs {
		"include",
		"ext"
	}
	files {
		"include/gslib/config.h",
		"src/gslib/error.cpp",
		"include/gslib/error.h",
		"src/gslib/md5.cpp",
		"include/gslib/md5.h",
		"include/gslib/pool.h",
		"src/gslib/res.cpp",
		"include/gslib/res.h",
		"include/gslib/std.h",
		"src/gslib/string.cpp",
		"include/gslib/string.h",
		"include/gslib/tree.h",
		"include/gslib/type.h",
		"src/gslib/xml.cpp",
		"include/gslib/xml.h",
		"proj/mres/main.cpp"
	}
	
project "uieditor"
	language "C++"
	kind "WindowedApp"
	entrypoint ""
	dependson {
		"gslib",
		"ariel"
	}
	includedirs {
		todir,
		"include",
		"src",
		"ext",
		"proj/uieditor"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"dxgi.lib",
		"d3d11.lib",
		"zlib.lib",
		"libjpeg.lib",
		"libpng.lib",
		"gslib.lib",
		"ariel.lib",
		"imm32.lib",
		"snmpapi.lib",
		"d3d10_1.lib",
		"d2d1.lib",
		"dwrite.lib"
	}
	files {
		"proj/uieditor/main.cpp",
		"proj/uieditor/basis.cpp",
		"proj/uieditor/uic.cpp",
		"proj/uieditor/basis.h",
		"proj/uieditor/uic.h",
		"proj/uieditor/io.cpp",
		"proj/uieditor/io.h"
	}
	
project "uic"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	dependson {
		"gslib",
		"ariel"
	}
	includedirs {
		todir,
		"include",
		"src",
		"ext",
		"proj/uieditor"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"dxgi.lib",
		"d3d11.lib",
		"zlib.lib",
		"libjpeg.lib",
		"libpng.lib",
		"gslib.lib",
		"ariel.lib",
		"imm32.lib",
		"snmpapi.lib",
		"d3d10_1.lib",
		"d2d1.lib",
		"dwrite.lib"
	}
	files {
		"proj/uieditor/uic/main.cpp",
		"proj/uieditor/basis.cpp",
		"proj/uieditor/uic.cpp",
		"proj/uieditor/basis.h",
		"proj/uieditor/uic.h",
		"proj/uieditor/io.cpp",
		"proj/uieditor/io.h"
	}
	
project "mayaplugin"
	language "C++"
	kind "SharedLib"
	entrypoint ""
	architecture "x86_64"
	targetextension ".mll"
	toolset "v140"
	inlining "Explicit"
	defines { "_USRDLL", "NT_PLUGIN", "_HAS_ITERATOR_DEBUGGING=0", "_SECURE_SCL=0", "_SECURE_SCL_THROWS=0", "_SECURE_SCL_DEPRECATE=0", "_CRT_SECURE_NO_DEPRECATE", "TBB_USE_DEBUG=0", "__TBB_LIB_NAME=tbb.lib", "REQUIRE_IOSTREAM", "AW_NEW_IOSTREAMS", "Bits64_" }		
	flags { "NoIncrementalLink" }
	stringpooling "on"
	exceptionhandling "on"
	staticruntime "off"
	functionlevellinking "on"
	rtti "on"
	buildoptions { "/export:initializePlugin", "/export:uninitializePlugin", "/DYNAMICBASE:NO" }
	includedirs {
		todir,
		"C:/Program Files/Autodesk/Maya2019/include",
		"D:/Program Files/Autodesk/Maya2019/include"
	}
	libdirs {
		"$(OutDir)",
		"C:/Program Files/Autodesk/Maya2019/lib",
		"D:/Program Files/Autodesk/Maya2019/lib"
	}
	links {
		"Foundation.lib",
		"OpenMaya.lib"
	}
	files {
		"proj/mayaplugin/main.cpp",
		"proj/mayaplugin/mesh.h",
		"proj/mayaplugin/mesh.cpp"
	}
	
project "copyrighttool"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	dependson {
		"zlib",
		"gslib"
	}
	includedirs {
		todir,
		"include",
		"ext"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"zlib.lib",
		"gslib.lib"
	}
	files {
		"proj/copyrighttool/main.cpp"
	}
	