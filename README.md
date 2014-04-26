# Debugler

## What?

The OpenGL 2.1+, OpenGL ES 2.0..3.1 debugger for Windows and Linux

## Platforms?
* Windows 32 or 64 bit, Vista or later
* Linux 32 or 64 bit
* Android (x86, arm or mips based). Tried 2.3.5 ... 4.4.2.

## Howto Build?

### Windows prerequisites

Needed: 
 * Microsoft Visual Studio 2012
 
 * Wix Toolset
 
 * CMake for windows, with cmake in <b>PATH</b>
 
 * Android NDK r8e, set <b>ANDROID_NDK</b> environment variable to NDK directory

 * Python 2.7.*, http://www.python.org/download/
 
 * LXML, http://www.lfd.uci.edu/~gohlke/pythonlibs/cua72n5h/lxml-3.2.4.win32-py2.7.exe

 * Qt 5.1.1, http://www.nic.funet.fi/pub/mirrors/download.qt-project.org/official_releases/qt/5.1/5.1.1/qt-windows-opensource-5.1.1-msvc2012-x86-offline.exe
 
 Debugler can build with <b>older versions</b>, but it <b> does not work </b> due to https://bugreports.qt-project.org/browse/QTBUG-29391

 * Set <b>QTDIR</b> environment variable to QT installation directory (like C:\Qt\Qt5.1.0\5.1.0-rc1\msvc2012)


### Linux (Ubuntu) prerequisites

Needed: 
  * Ubuntu packages: g++ x11proto-gl-dev libelf-dev cmake lxml libqt4-dev
   
  * Android NDK r8e, set <b>ANDROID_NDK</b> environment variable to NDK directory
 

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

####Development
### Code style
  * Use clanf-format from llvm project, use provided .clang_format definitions. 
   * http://llvm.org/builds/downloads/ClangFormat.vsix
   * http://llvm.org/builds/downloads/LLVM-3.4.r194000-win32.exe

