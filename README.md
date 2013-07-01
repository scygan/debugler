[![Build Status](https://secure.travis-ci.org/debugler/debugler.png?branch=master)](http://travis-ci.org/debugler/debugler)
[![Ohloh badge](https://www.ohloh.net/p/debugler/widgets/project_thin_badge.gif)](http://www.ohloh.net/p/debugler)


# Debugler

## What?

The OpenGL 2.1+, OpenGL ES 2.0/3.0 debugger for Windows and Linux

## Platforms?
* Windows 32/64 bit, Vista or later
* Linux  

## Howto Build?

### Build for Windows

Needed: 
 * Microsoft Visual Studio 2012

 * Python 2.7.*, http://www.python.org/download/

 * Qt 5.0.1-rc1, http://download.qt-project.org/development_releases/qt/5.1/5.1.0-rc1/qt-windows-opensource-5.1.0-rc1-msvc2012-x86-offline.exe

 * Set <b>QTDIR</b> environment variable to QT installation directory (like C:\Qt\Qt5.1.0\5.1.0-rc1\msvc2012).

 * Run 
```
src\built.bat
```
 * Installers are build in <b>build\Win32\Release\Installer</b> and <b>build\x64\Release\Installer</b> directories

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

