# gslib
Everything from scratch, just for fun

For start:
Use the gsmake.exe to generate project file.
A typical command line was like:

gsmake -debug -compiler "vs_12" -tar "c:/"

Only Visual Studio 2013 and 2015 was currently supported,
otherwise modify gsm.h|.cpp to generate the project file you wanted, DIY ;-)
or simply use your own ways to make these codes running, if you knew the rules.

for Visual Studio 2015, use option -compiler "vs_14"
for release, use option -release instead of -debug
for the source directory, use option -src "", default by current directory

Notice:
These codes were experimentally for now. You may read it for fun, but be ware
about the risks if you wanted to use them in an actual project.

2016.10.18
About the compile error under Visual Studio 2015
They may caused by the linkage error of CxIMAGE libs, just remove them for now,
they were useless and was going to be removed in the future.
