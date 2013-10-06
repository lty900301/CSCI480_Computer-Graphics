USC CSCI480 Computer Graphics
=========================
Author: Josh Luo

1. Assignment 1 - Height Field
------------

<dl>
  <dt>Compile instruction for Mac OS X</dt>
  <dd>For Mac OS X, you will  need the [pic image library](https://github.com/lty900301/CSCI480_Computer-Graphics/tree/master/pic). The makefile of the starter code assumes that the pic library locates one level above (i.e. if starter code == /Users/tom/code/assign1, then pic library should be in /Users/tom/code/pic). Please compile the pic library before compiling the starter code. Here is a sample sequence of commands that get everything compiled:</dd>
</dl>
```
unzip pic_MacOS.zip
unzip assign1_starterCode_macOS.zip
cd pic 
export CPPFLAGS=-I/opt/X11/include
make 
cd ..
cd assign1
make
./assign1 spiral.jpg
```