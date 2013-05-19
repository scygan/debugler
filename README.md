[![Build Status](https://secure.travis-ci.org/debugler/debugler.png?branch=master)](http://travis-ci.org/debugler/debugler)

# Debugler

## What?

The OpenGL 2.1+, OpenGL ES 2.0/3.0 debugger for Windows and Linux

## Platforms?
* Windows 32/64 bit, Vista or later
* Linux  

## Howto Build?

### Build for Windows

Needed: 
 * Microsoft Visual Studio 2010 (non-express edition). No 2012 support yet.

 * Visual Studio Add-in 1.1.11 for Qt4 (http://releases.qt-project.org/vsaddin/qt-vs-addin-1.1.11-opensource.exe)

 * Python 2.7.*, http://www.python.org/download/

 * Qt 4.8.4, http://download.qt-project.org/official_releases/qt/4.8/4.8.4/qt-win-opensource-4.8.4-vs2010.exe

 * Set <b>QTDIR</b> environment variable to QT installation directory 

 * Run 
```
src\built.bat
```
 * Installer is build in <b>src\built.bat</b> directory

####Running unit tests

 * Install python setup tools: https://pypi.python.org/pypi/setuptools/

 * Install required modules:
```
C:\Python27\scripts\easy_install pyopengl
```
 * Test python install by running <b>debugler\src\tests\samples\simple.py</b>
   On my machine i had this problem: http://stackoverflow.com/questions/10188595/pyopengl-typeerror-nonetype-object-is-not-callable
   Resolved by copying missing DLLs as described

 * Run
  ```
  build\Win32\Release\UT\ut.exe
  ```

### Build for Linux

  * Install (for Ubuntu): g++-4.7 x11proto-gl-dev libelf-dev python-opengl
  * Run
```
   cmake . -DCMAKE_BUILD_TYPE=Release && make package
```
  * Install built deb:
```  
  sudo dpkg -i debugler-...-Linux.deb
```
  * Run
```  
   debugler
```

####Running unit tests

  * Install (for Ubuntu): python-opengl
  * Run
```
  ./tests/UT/ut
```

