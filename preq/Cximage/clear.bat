@echo off
del /f /q /s *.suo *.sdf *.opensdf *.ncb *.old *.user
del /f /q /s /a:h *.suo *.sdf *.opensdf *.ncb *.old *.user
rd /q /s bin
rd /q /s debug
rd /q /s release
rd /q /s "CxImage/debug"
rd /q /s "CxImage/release"
rd /q /s "CxImage/CxImageDLL/debug"
rd /q /s "CxImage/CxImageDLL/release"
rd /q /s "demo/debug"
rd /q /s "demo/DebugDll"
rd /q /s "demo/release"
rd /q /s "demo/ReleaseDll"
rd /q /s "jasper/debug"
rd /q /s "jasper/release"
rd /q /s "jbig/debug"
rd /q /s "jbig/release"
rd /q /s "jpeg/debug"
rd /q /s "jpeg/release"
rd /q /s "libpsd/debug"
rd /q /s "libpsd/release"
rd /q /s "mng/debug"
rd /q /s "mng/release"
rd /q /s "png/debug"
rd /q /s "png/release"
rd /q /s "raw/debug"
rd /q /s "raw/release"
rd /q /s "tiff/debug"
rd /q /s "tiff/release"
rd /q /s "zlib/debug"
rd /q /s "zlib/release"
