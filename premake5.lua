local action = _ACTION or ""
local todir = "build/" .. action

solution "gslib"
	configurations {
		"Debug",
		"Release"
	}
	location (todir)
	
	-- no rtti
	rtti "Off"
	
	-- debug configs
	configuration "Debug"
		defines { "DEBUG" }
		symbols "On"

	-- release configs
	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }
		
	-- windows specific
	configuration "windows"
		defines { "WIN32", "_WINDOWS", "_UNICODE" }
		
project "ariel"
	language "C++"
	kind "WindowedApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		todir,
		"code",
		"code/ext"
	}
	libdirs {
		"code"
	}
	links {
		"dxgi.lib",
		"d3d11.lib",
		"d3dcompiler.lib"
	}
	prebuildcommands {
		'fxc /T vs_4_0 /E "rose_vsf_cr" /Fd /Zi /Fh "rose_vsf_cr.h" "../../code/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_psf_cr" /Fd /Zi /Fh "rose_psf_cr.h" "../../code/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vsf_klm_cr" /Fd /Zi /Fh "rose_vsf_klm_cr.h" "../../code/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_psf_klm_cr" /Fd /Zi /Fh "rose_psf_klm_cr.h" "../../code/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vss_coef_cr" /Fd /Zi /Fh "rose_vss_coef_cr.h" "../../code/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_pss_coef_cr" /Fd /Zi /Fh "rose_pss_coef_cr.h" "../../code/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vsf_klm_tex" /Fd /Zi /Fh "rose_vsf_klm_tex.h" "../../code/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_psf_klm_tex" /Fd /Zi /Fh "rose_psf_klm_tex.h" "../../code/ariel/rose.hlsl"',
		'fxc /T vs_4_0 /E "rose_vss_coef_tex" /Fd /Zi /Fh "rose_vss_coef_tex.h" "../../code/ariel/rose.hlsl"',
		'fxc /T ps_4_0 /E "rose_pss_coef_tex" /Fd /Zi /Fh "rose_pss_coef_tex.h" "../../code/ariel/rose.hlsl"'
	}
	files {
		"code/ariel/batch.cpp",
		"code/ariel/batch.h",
		"code/pink/clip.cpp",
		"code/pink/clip.h",
		"code/gslib/config.h",
		"code/pink/config.h",
		"code/ariel/config.h",
		"code/ariel/delaunay.cpp",
		"code/ariel/delaunay.h",
		"code/pink/dirty.cpp",
		"code/pink/dirty.h",
		"code/gslib/entrywin32.cpp",
		"code/gslib/entrywin32.h",
		"code/gslib/error.cpp",
		"code/gslib/error.h",
		"code/ariel/framesys.cpp",
		"code/ariel/framesys.h",
		"code/ariel/framesyswin32.cpp",
		"code/ariel/mesh.cpp",
		"code/ariel/mesh.h",
		"code/ariel/mesh.hpp",
		"code/pink/image.cpp",
		"code/pink/image.h",
		"code/pink/imageio.cpp",
		"code/pink/imageio.h",
		"code/ariel/loopblinn.cpp",
		"code/ariel/loopblinn.h",
		"proj/ariel/main.cpp",
		"code/gslib/math.cpp",
		"code/gslib/math.h",
		"code/gslib/math.inl",
		"code/gslib/mclass.h",
		"code/pink/painter.cpp",
		"code/pink/painter.h",
		"code/gslib/pool.cpp",
		"code/gslib/pool.h",
		"code/ariel/rectpack.cpp",
		"code/ariel/rectpack.h",
		"code/pink/raster.cpp",
		"code/pink/raster.h",
		"code/ariel/rendersys.cpp",
		"code/ariel/rendersys.h",
		"code/ariel/rendersysd3d11.cpp",
		"code/ariel/rendersysd3d11.h",
		"code/ariel/rose.cpp",
		"code/ariel/rose.h",
		"code/ariel/rose.hlsl",
		"code/gslib/rtree.h",
		"code/ariel/texbatch.h",
		"code/ariel/texbatch.cpp",
		"code/ariel/scene.cpp",
		"code/ariel/scene.h",
		"code/gslib/std.h",
		"code/gslib/string.cpp",
		"code/gslib/string.h",
		"code/gslib/sysop.h",
		"code/gslib/tree.h",
		"code/pink/type.cpp",
		"code/pink/type.h",
		"code/gslib/type.h",
		"code/ariel/type.h",
		"code/pink/utility.cpp",
		"code/pink/utility.h",
		"code/pink/widget.cpp",
		"code/pink/widget.h"
	}
	
project "rathen"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
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
		"code/gslib/pool.cpp",
		"proj/rathen/main.cpp"
	}
	
-- following are tests
project "avl"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"code/gslib/avl.h",
		"code/gslib/rbtree.h",
		"code/gslib/config.h",
		"code/gslib/error.cpp",
		"code/gslib/error.h",
		"code/gslib/file.h",
		"code/gslib/math.cpp",
		"code/gslib/math.h",
		"code/gslib/math.inl",
		"code/gslib/mtrand.cpp",
		"code/gslib/mtrand.h",
		"code/gslib/pool.cpp",
		"code/gslib/pool.h",
		"code/gslib/std.h",
		"code/gslib/string.cpp",
		"code/gslib/string.h",
		"code/gslib/sysop.h",
		"code/gslib/type.h",
		"test/avl/main.cpp"
	}
	
project "cdt"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"code/pink/clip.cpp",
		"code/ariel/delaunay.cpp",
		"code/pink/dirty.cpp",
		"code/gslib/error.cpp",
		"code/pink/image.cpp",
		"code/pink/imageio.cpp",
		"code/gslib/math.cpp",
		"code/gslib/mtrand.cpp",
		"code/pink/painter.cpp",
		"code/gslib/pool.cpp",
		"code/pink/raster.cpp",
		"code/gslib/res.cpp",
		"code/gslib/string.cpp",
		"code/pink/type.cpp",
		"code/pink/utility.cpp",
		"test/cdt/main.cpp"
	}
	
project "rtree"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"code/gslib/config.h",
		"code/gslib/error.cpp",
		"code/gslib/error.h",
		"code/gslib/math.cpp",
		"code/gslib/math.h",
		"code/gslib/math.inl",
		"code/gslib/pool.cpp",
		"code/gslib/pool.h",
		"code/gslib/rtree.h",
		"code/gslib/std.h",
		"code/gslib/string.cpp",
		"code/gslib/string.h",
		"code/gslib/tree.h",
		"code/gslib/treeop.h",
		"code/pink/type.cpp",
		"code/pink/type.h",
		"test/rtree/main.cpp"
	}
	
project "rectpack"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"code/gslib/config.h",
		"code/gslib/error.cpp",
		"code/gslib/error.h",
		"code/gslib/pool.cpp",
		"code/gslib/pool.h",
		"code/gslib/std.h",
		"code/gslib/string.cpp",
		"code/gslib/string.h",
		"code/gslib/tree.h",
		"code/gslib/type.h",
		"code/pink/type.cpp",
		"code/pink/type.h",
		"code/ariel/config.h",
		"code/ariel/type.h",
		"code/ariel/rectpack.cpp",
		"code/ariel/rectpack.h",
		"test/rectpack/main.cpp"
	}

-- following are tools
project "mres"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"code/gslib/config.h",
		"code/gslib/error.cpp",
		"code/gslib/error.h",
		"code/gslib/md5.cpp",
		"code/gslib/md5.h",
		"code/gslib/pool.cpp",
		"code/gslib/pool.h",
		"code/gslib/res.cpp",
		"code/gslib/res.h",
		"code/gslib/std.h",
		"code/gslib/string.cpp",
		"code/gslib/string.h",
		"code/gslib/tree.h",
		"code/gslib/type.h",
		"code/gslib/xml.cpp",
		"code/gslib/xml.h",
		"proj/mres/main.cpp"
	}
	
project "copyrighttool"
	language "C++"
	kind "ConsoleApp"
	entrypoint ""
	if _ACTION == "vs2017" then
		sysincludedirs {
			"C:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt"
		}
		syslibdirs {
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/ucrt/x86"
		}
	end
	includedirs {
		"code",
		"code/ext"
	}
	files {
		"proj/copyrighttool/main.cpp"
	}
	