#gslib

Everything from scratch, just for fun

For start:<br>
Use premake5 to make the project file.<br>
For premake5, pay a visit to:<br>
[http://premake.github.io/](http://premake.github.io/)<br>
<br>
A typical command line was like:<br>

```
premake5 vs2013
```

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
