%windir%\Microsoft.NET\Framework\v4.0.30319\MSBuild debugler.sln /p:VisualStudioVersion=11.0 /m /nologo /t:Build /p:Configuration=Release;platform=x64
if %errorlevel% neq 0 exit /b %errorlevel% 
%windir%\Microsoft.NET\Framework\v4.0.30319\MSBuild debugler.sln /p:VisualStudioVersion=11.0 /m /nologo /t:Build /p:Configuration=Release;platform=Win32
if %errorlevel% neq 0 exit /b %errorlevel% 
"%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\Common7\IDE\devenv.exe" buildtools\vs\auxprojects\Installer\Installer.vdproj /build Installer