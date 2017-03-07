#gslib

Everything from scratch, just for fun

For start:  
Use premake5 to make the project file.  
  
For premake5, pay a visit to:  
[http://premake.github.io/](http://premake.github.io/)  
  
A typical command line was like:  

```
premake5 vs2013
```

Notice:  
These codes were experimentally for now. You may read it for fun, but be ware  
about the risks if you wanted to use them in an actual project.  

For the next:  
The lines in vector arts and graphs to be drawn would be anti-aliased,  
I'm about to add AA functions to do this, along with a stroke batcher  

And in the future,  
libpng, libjpeg, etcs would be directly used by the project, thus removing the  
dependancy of CxImage.  

Quite a lot of other funny things would be added to the GSLIB, too ;-)  

2016.10.18  
About the compile error under Visual Studio 2015  
They may caused by the linkage error of CxImage libs, just remove them for now,  
they were useless and was going to be removed in the future.  
