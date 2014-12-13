# Debugler

## What?

The OpenGL 2.1+, OpenGL ES 2.0..3.1 debugger for Windows and Linux

## Platforms?
* Windows 32 or 64 bit, Vista or later
* Linux 32 or 64 bit
* Android (x86, arm or mips). Tried 2.3.5 ... 5.0.

## Howto Build?

### Windows prerequisites

Needed: 
 * Microsoft Visual Studio 2013
 
 * Wix Toolset
 
 * CMake for windows, with cmake in <b>PATH</b>
 
 * Android NDK r9b, http://dl.google.com/android/ndk/android-ndk-r9b-windows-x86.exe, set <b>ANDROID_NDK</b> environment variable to NDK directory

 * Python 2.7.*, http://www.python.org/download/
 
 * LXML, http://www.lfd.uci.edu/~gohlke/pythonlibs/#lxml

 * Qt 5.4.0 for Visual Studio 2013, https://www.qt.io/download-open-source/

 * Set <b>QTDIR</b> environment variable to QT installation directory (like D:\Qt\Qt5.4.0\5.4\msvc2013)
 
 * Visual Studio Add-in for QT5, https://www.qt.io/download-open-source


### Linux (Ubuntu) prerequisites

Needed: 
  * Ubuntu packages: g++ x11proto-gl-dev libelf-dev cmake libxml2-dev libqt4-dev
   
  * Android NDK r9b, set <b>ANDROID_NDK</b> environment variable to NDK directory
 

### Building Debugler:

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
   * <b>android-x86</b>: just android-arm binary
   * <b>android-mips</b>: just android-arm binary
  

####Running unit tests

 * Run
```
cd build/<target>/<configuration>/UT
ut
```

 * Under Visual Studio you can use *GoogleTest Runner* extension

### Development
#### Code style
  * Use clanf-format from llvm project, use provided .clang_format definitions. 
   * http://llvm.org/builds/downloads/ClangFormat.vsix
   * http://llvm.org/builds/downloads/LLVM-3.4.r194000-win32.exe

