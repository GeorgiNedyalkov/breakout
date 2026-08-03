@echo off

mkdir ..\build
pushd ..\build
cl -Zi "..\src\main.c" /I"..\resources\raylib\include" ^
	/Fe"main.exe" ^
	/link /libpath:"..\resources\raylib\lib" ^
	gdi32.lib msvcrt.lib raylib.lib winmm.lib user32.lib shell32.lib /DEBUG ^
	/NODEFAULTLIB:libcmt /NODEFAULTLIB:msvcrtd
popd

