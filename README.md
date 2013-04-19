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

 * Qt 4.8.3, compiled with static runtime:
   * Get http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.zip,

   * Unpack, edit qt-everywhere-opensource-src-4.8.4\mkspecs\win32-msvc2010:

   * Replace 
   
```
QMAKE_CFLAGS_RELEASE    = -O2 -MD
QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -O2 -MD -Zi
QMAKE_CFLAGS_DEBUG      = -Zi -MDd
```

   with
   
```
QMAKE_CFLAGS_RELEASE    = -O2 -MT
QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -O2 -MT -Zi
QMAKE_CFLAGS_DEBUG      = -Zi -MTd
```

   * Build:
   
```
"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\vcvarsall.bat"
cd qt-everywhere-opensource-src-4.8.4
configure.exe -debug-and-release -platform win32-msvc2010 -no-qt3support -fast
nmake
```

 * Set <b>QTDIR</b> environment variable to location of <b>qt-everywhere-opensource-src-4.8.4</b> directory

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

