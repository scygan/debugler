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
 
 * Wix Toolset
 
 * CMake for windows, with cmake in <b>PATH</b>
 
 * Android NDK r8e

 * Python 2.7.*, http://www.python.org/download/
 
 * LXML, http://www.lfd.uci.edu/~gohlke/pythonlibs/cua72n5h/lxml-3.2.4.win32-py2.7.exe

 * Qt 5.1.1, http://www.nic.funet.fi/pub/mirrors/download.qt-project.org/official_releases/qt/5.1/5.1.1/qt-windows-opensource-5.1.1-msvc2012-x86-offline.exe
 
 Debugler can build with <b>older versions</b>, but it <b> does not work </b> due to https://bugreports.qt-project.org/browse/QTBUG-29391

 * Set <b>QTDIR</b> environment variable to QT installation directory (like C:\Qt\Qt5.1.0\5.1.0-rc1\msvc2012), set <b>ANDROID_NDK</b> environment variable to NDK directory

 * Run 
```
src\built.bat
```
 * Installers are build in <b>build\Win32\Release\Installer</b> and <b>build\x64\Release\Installer</b> directories

####Running unit tests

 * Run
  ```
  cd build\Win32\Release\
  UT\ut.exe
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

####Development
### Code style
Use clanf-format from llvm project, use provided .clang_format definitions. 
http://llvm.org/builds/downloads/ClangFormat.vsix
http://llvm.org/builds/downloads/LLVM-3.4.r194000-win32.exe

