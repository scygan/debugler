[![Build Status](https://travis-ci.org/scygan/debugler.svg?branch=master)](https://travis-ci.org/scygan/debugler)	

# Debugler

## What?

The OpenGL 2.1+, OpenGL ES 2.0+ mutliplatform debugger

## Supported platforms
Currently you can debug OpenGL applications on following systems:
* Windows (Vista or newer, 32- and 64-bit applications)
* Linux (tried x86 and x86_64 applications on Ubuntu 12.04)
* Android (x86, x86_64, ARM or MIPS). Tried versions 2.3.5 through 5.0.

Additionally, the GUI runs on Windows and Linux platforms.

## Build instructions

Building using provided scripts is yeasy, however thery are multiple tool 
and library prerequisites that must be installed before the build.

Following lists the best known versions of these tools and libraries.

### Windows prerequisites

 * Microsoft Visual Studio 2013 (Community Edition, or other version other than Express).
 
 * Wix Toolset
 
 * CMake 3.3 or later for windows, with cmake in <b>PATH</b> environment variable
 
 * Android NDK r10d, http://dl.google.com/android/ndk/, set <b>ANDROID_NDK</b> environment variable to NDK directory

 * Python 2.7.*, http://www.python.org/download/
 
 * LXML, http://www.lfd.uci.edu/~gohlke/pythonlibs/#lxml

 * Qt 5.7.0 for Visual Studio 2013 32-bit, https://www.qt.io/download-open-source/

 * Set <b>QTDIR</b> environment variable to QT installation directory (like D:\Qt\Qt5.4.0\5.4\msvc2013)
 
 * Visual Studio Add-in for QT5, https://www.qt.io/download-open-source


### Linux prerequisites

On Ubuntu 12.04, following packages are needed

Needed: 
  * Ubuntu packages: cmake g++-4.7 libxxf86vm-dev python-lxml x11proto-gl-dev libelf-dev libqt4-dev (or QT 5.0 version)
   
  * Android NDK r9b, set <b>ANDROID_NDK</b> environment variable to NDK directory
 

### Building Debugler:

Just run the script:

```
built.py
```
 
  * By default build.py will build 64-bit (x86_64) redistributable installable package. On Windows this is an MSI installer, on Ubuntu this is a debugler-*-Linux-*deb package.
  
  * Installers are build in <b>build\x64\Release\Installer</b> (on Windows) or  <b>build/64-dist/Release</b> (on Linux) directories.

  * Other targets may be build using following command line:

```
built.py [target]
```

  * Available targets: 
   * <b>64-dist</b>: *default*, 64-bit installer/deb package, with all Android binaries
   * <b>32-dist</b>: 32-bit installer/deb package, with all Android binaries
   * <b>64</b>: 64-bit installer/deb package
   * <b>32</b>: 32-bit installer/deb package
   * <b>android-arm</b>: just android-arm binary (dglandroidinstaller)
   * <b>android-x86</b>: just android-x86 binary
   * <b>android-mips</b>: just android-mips binary
  

####Running unit tests

 * Run
```
cd build/<target>/<configuration>/UT
ut
```

 * Under Visual Studio you can use *GoogleTest Runner* extension

### Development
#### Code style
  * Use clang-format from llvm project, use provided .clang_format definitions. 
   * http://llvm.org/builds/downloads/ClangFormat.vsix
   * http://llvm.org/builds/downloads/LLVM-3.4.r194000-win32.exe

