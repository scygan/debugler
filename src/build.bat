IF "%DGL_VERSION%"=="" (SET DGL_VERSION=0.0.0-ENG)
%windir%\Microsoft.NET\Framework\v4.0.30319\MSBuild debugler.sln /p:VisualStudioVersion=11.0 /m /nologo /t:Build /p:Configuration=Release;platform=Win32
if %errorlevel% neq 0 exit /b %errorlevel% 
%windir%\Microsoft.NET\Framework\v4.0.30319\MSBuild debugler.sln /p:VisualStudioVersion=11.0 /m /nologo /t:Build /p:Configuration=Release;platform=x64