rem Собрать проект
echo Building...
cd %~dp0
del /F /Q build
mkdir build
mkdir build\images
copy images build\images\
cd build
cmake ..

echo Build complete!

echo Detecting Visual Studio...
rem Обнаружить visual studio
if exist "%VS140COMNTOOLS%"vsvars32.bat (
	goto vs14
)
if exist "%VS130COMNTOOLS%"vsvars32.bat (
	goto vs13
)
if exist "%VS120COMNTOOLS%"vsvars32.bat (
	goto vs12
)
if exist "%VS110COMNTOOLS%"vsvars32.bat (
	goto vs11
)
if exist "%VS100COMNTOOLS%"vsvars32.bat (
	goto vs10
)

echo [E]: Can't detect Visual Studio!
pause
exit

rem Настроить окружение
:vs13
call "%VS140COMNTOOLS%"vsvars32.bat
goto compile
:vs13
call "%VS130COMNTOOLS%"vsvars32.bat
goto compile
:vs12
call "%VS120COMNTOOLS%"vsvars32.bat
goto compile
:vs11
call "%VS120COMNTOOLS%"vsvars32.bat
goto compile
:vs10
call "%VS120COMNTOOLS%"vsvars32.bat
goto compile

rem Скомпилировать
:compile
echo "Compilation.."
devenv cl.sln /build "Release|Win32" 
rem devenv cl.sln /build "Debug|Win32"
rem devenv cl.sln /build "Release|x64"
rem devenv cl.sln /build "Debug|x64"

echo [MSVC] FINISHED!
pause
exit
