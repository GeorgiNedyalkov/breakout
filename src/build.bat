@echo off

mkdir ..\build
pushd ..\build
cl -Zi "..\src\main.c" /I"C:\lib\raylib-6.0_win64_msvc16\include" ^
	/Fe"main.exe" ^
	/link /libpath:"C:\lib\raylib-6.0_win64_msvc16\lib" ^
	gdi32.lib msvcrt.lib raylib.lib winmm.lib user32.lib shell32.lib /DEBUG ^
	/NODEFAULTLIB:libcmt /NODEFAULTLIB:msvcrtd
popd

