set PATH=%PATH%;C:\Python27
mkdir ..\build
mkdir ..\build\android-arm
cd ..\build\android-arm
if %errorlevel% neq 0 goto error
cmake -DCMAKE_TOOLCHAIN_FILE=..\..\src\android.toolchain.cmake ..\..\src\ -G "NMake Makefiles"
jom


:error
cd ..\..\src
exit /b 1
