local action = _ACTION or ""
local todir = "build/" .. action

solution "gslib_miscs"
	configurations {
		"Debug",
		"Release"
	}
	location (todir)
	
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

project "test111"
	language "C++"
	kind "WindowedApp"
	entrypoint ""
	dependson {
		"zlib",
		"libpng",
		"libjpeg",
		"gslib",
		"ariel"
	}
	includedirs {
		todir,
		"include",
		"src",
		"ext"
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
		"snmpapi.lib"
	}
	files {
		"proj/ariel/main.cpp"
	}
	
project "widget"
	language "C++"
	kind "WindowedApp"
	entrypoint ""
	dependson {
		"zlib",
		"libpng",
		"libjpeg",
		"gslib",
		"ariel"
	}
	includedirs {
		todir,
		"include",
		"src",
		"ext"
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
		"test/widget/main.cpp"
	}
	
-- following are tests
project "avl"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	includedirs {
		"include",
		"ext"
	}
	files {
		"include/gslib/avl.h",
		"include/gslib/rbtree.h",
		"include/gslib/config.h",
		"include/gslib/mtrand.h",
		"src/gslib/error.cpp",
		"include/gslib/error.h",
		"include/gslib/file.h",
		"src/gslib/math.cpp",
		"include/gslib/math.h",
		"include/gslib/math.inl",
		"include/gslib/pool.h",
		"include/gslib/std.h",
		"src/gslib/string.cpp",
		"include/gslib/string.h",
		"include/gslib/type.h",
		"src/gslib/mtrand.cpp",
		"test/avl/main.cpp"
	}
	
project "cdt"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	dependson {
		"zlib",
		"libpng",
		"libjpeg",
		"gslib",
		"ariel"
	}
	includedirs {
		"include",
		"ext"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"zlib.lib",
		"libjpeg.lib",
		"libpng.lib",
		"gslib.lib",
		"ariel.lib",
		"d3d10_1.lib",
		"dwrite.lib",
		"d2d1.lib"
	}
	files {
		"test/cdt/main.cpp"
	}
	
project "rtree"
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
		"src/gslib/math.cpp",
		"include/gslib/math.h",
		"include/gslib/math.inl",
		"include/gslib/pool.h",
		"include/gslib/rtree.h",
		"include/gslib/std.h",
		"src/gslib/string.cpp",
		"include/gslib/string.h",
		"include/gslib/tree.h",
		"include/gslib/treeop.h",
		"src/gslib/type.cpp",
		"include/gslib/type.h",
		"test/rtree/main.cpp"
	}
	
project "rectpack"
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
		"include/gslib/pool.h",
		"include/gslib/std.h",
		"src/gslib/string.cpp",
		"include/gslib/string.h",
		"include/gslib/tree.h",
		"include/gslib/type.h",
		"src/gslib/type.cpp",
		"include/ariel/config.h",
		"include/ariel/type.h",
		"src/ariel/rectpack.cpp",
		"include/ariel/rectpack.h",
		"test/rectpack/main.cpp"
	}

project "testfreetype"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	dependson {
		"freetype"
	}
	includedirs {
		"include",
		"ext/freetype/include"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"freetype.lib"
	}
	files {
		"test/testfreetype/main.cpp"
	}
	
project "texpack"
	language "C++"
	kind "WindowedApp"
	entrypoint ""
	dependson {
		"libpng",
		"libjpeg",
		"zlib",
		"gslib",
		"ariel"
	}
	includedirs {
		"include",
		"framework"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"libpng.lib",
		"libjpeg.lib",
		"zlib.lib",
		"gslib.lib",
		"ariel.lib",
		"dxgi.lib",
		"imm32.lib",
		"d3d11.lib",
		"snmpapi.lib",
		"d3d10_1.lib",
		"dwrite.lib",
		"d2d1.lib"
	}
	files {
		"test/texpack/main.cpp",
		"framework/entrywin32.cpp",
		"framework/entrywin32.h"
	}
	
project "mkraster"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	dependson {
		"libpng",
		"libjpeg",
		"zlib",
		"gslib",
		"ariel",
		"mikan"
	}
	includedirs {
		"include",
		"ext"
	}
	libdirs {
		"$(OutDir)"
	}
	links {
		"libpng.lib",
		"libjpeg.lib",
		"zlib.lib",
		"gslib.lib",
		"ariel.lib",
		"dxgi.lib",
		"imm32.lib",
		"d3d11.lib",
		"snmpapi.lib",
		"mikan.lib"
	}
	files {
		"test/mkraster/main.cpp"
	}

project "msomorph"
	language "C++"
	kind "WindowedApp"
	entrypoint ""
	dependson {
		"zlib",
		"libpng",
		"libjpeg",
		"gslib",
		"ariel"
	}
	includedirs {
		todir,
		"include",
		"src",
		"ext",
		"framework"
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
		"dwrite.lib",
		"d2d1.lib"
	}
	prebuildcommands {
		'fxc /T vs_4_0 /E "vs_main" /Fd /Zi /Fh "vs_main.h" "../../proj/msomorph/shader.hlsl"',
		'fxc /T ps_4_0 /E "ps_main" /Fd /Zi /Fh "ps_main.h" "../../proj/msomorph/shader.hlsl"'
	}
	files {
		"proj/msomorph/main.cpp"
		--"proj/msomorph/shader.hlsl"
	}
	