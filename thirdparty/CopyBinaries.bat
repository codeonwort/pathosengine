@echo off
setlocal

set SolutionDir=%1
set SolutionConfig=%2

echo SolutionDir=%SolutionDir%
echo Configuration=%SolutionConfig%

if not exist "%SolutionDir%\PathosEngine.sln" (
	echo First argument should be the solution directory, but couldn't find PathosEngine.sln
	exit
)

set ValidConfig=0
if "%SolutionConfig%" == "Debug" set ValidConfig=1
if "%SolutionConfig%" == "Release" set ValidConfig=1
if %ValidConfig%==0 (
	echo Second argument should be Debug or Release
	exit
)

if "%SolutionConfig%" == "Debug" set FreeGLUTFile=freeglutd.dll
if "%SolutionConfig%" == "Release" set FreeGLUTFile=freeglut.dll

xcopy /y /d "%SolutionDir%\thirdparty\freeglut\binary\%FreeGLUTFile%" "%SolutionDir%\bin\%SolutionConfig%\"
xcopy /y /d "%SolutionDir%\thirdparty\freeimage-3.18.0\binary\freeimage.dll" "%SolutionDir%\bin\%SolutionConfig%\"
xcopy /y /d "%SolutionDir%\thirdparty\bass\build\x64\bass.dll" "%SolutionDir%\bin\%SolutionConfig%\"

endlocal
