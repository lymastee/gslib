@echo off
cd debug
copy "cximage.lib" "../../code/extern/cximage"
copy "jpeg.lib" "../../code/extern/cximage"
copy "png.lib" "../../code/extern/cximage"
copy "tiff.lib" "../../code/extern/cximage"
copy "zlib.lib" "../../code/extern/cximage"
copy "cximage.pdb" "../../code/extern/cximage"
copy "jpeg.pdb" "../../code/extern/cximage"
copy "png.pdb" "../../code/extern/cximage"
copy "tiff.pdb" "../../code/extern/cximage"
copy "zlib.pdb" "../../code/extern/cximage"
cd "../../code/extern/cximage"
del /q /s /f cximage_d.lib jpeg_d.lib png_d.lib tiff_d.lib zlib_d.lib cximage_d.pdb jpeg_d.pdb png_d.pdb tiff_d.pdb zlib_d.pdb
ren cximage.lib cximage_d.lib
ren jpeg.lib jpeg_d.lib
ren png.lib png_d.lib
ren tiff.lib tiff_d.lib
ren zlib.lib zlib_d.lib
ren cximage.pdb cximage_d.pdb
ren jpeg.pdb jpeg_d.pdb
ren png.pdb png_d.pdb
ren tiff.pdb tiff_d.pdb
ren zlib.pdb zlib_d.pdb
cd "../../../preq/release"
copy "cximage.lib" "../../code/extern/cximage"
copy "jpeg.lib" "../../code/extern/cximage"
copy "png.lib" "../../code/extern/cximage"
copy "tiff.lib" "../../code/extern/cximage"
copy "zlib.lib" "../../code/extern/cximage"
cd "../cximage/cximage"
copy "stdint.h" "../../../code/extern/cximage"
copy "xfile.h" "../../../code/extern/cximage"
copy "ximacfg.h" "../../../code/extern/cximage"
copy "ximadef.h" "../../../code/extern/cximage"
copy "ximage.h" "../../../code/extern/cximage"
copy "xiofile.h" "../../../code/extern/cximage"
copy "xmemfile.h" "../../../code/extern/cximage"