#gslib

Everything from scratch, just for fun

For start:<br>
Use the gsmake.exe to generate project file.<br>
A typical command line was like:<br>

```
gsmake -debug -compiler "vs_12" -tar "c:/"
```

Only Visual Studio 2013 and 2015 was currently supported,<br>
otherwise modify gsm.h|.cpp to generate the project file you wanted, DIY ;-)<br>
or simply use your own ways to make these codes running, if you knew the rules.<br>

for Visual Studio 2015, use option -compiler "vs_14".<br>
for release, use option -release instead of -debug.<br>
for the source directory, use option -src "", default by current directory.<br>

Notice:<br>
These codes were experimentally for now. You may read it for fun, but be ware<br>
about the risks if you wanted to use them in an actual project.<br>

For the next:<br>
The lines in vector arts and graphs to be drawn would be anti-aliased,<br>
I'm about to add AA functions to do this, along with a stroke batcher<br>

and in the future,<br>
libpng, libjpeg, etcs would be directly used by the project, thus removing the<br>
dependancy of CxImage.<br>

Quite a lot of other funny things would be added to the GSLIB, too ;-)<br>

2016.10.18<br>
About the compile error under Visual Studio 2015<br>
They may caused by the linkage error of CxImage libs, just remove them for now,<br>
they were useless and was going to be removed in the future.<br>
