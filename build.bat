
@echo off
setlocal
set "MSYS2_PATH=C:\msys64\usr\bin;C:\msys64\ucrt64\bin"
set "PYTHON2_PATH=C:\Python27"
set "PATH=%PYTHON2_PATH%;%MSYS2_PATH%;%PATH%"

@REM make deps PLAT=WINDOWS > build.log 2>&1
make pf PLAT=WINDOWS > build.log 2>&1
make launchers PLAT=WINDOWS >> build.log 2>&1
copy /y "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" .\lib\ >nul 2>&1
copy /y "C:\msys64\ucrt64\bin\libstdc++-6.dll" .\lib\ >nul 2>&1
copy /y "C:\msys64\ucrt64\bin\libwinpthread-1.dll" .\lib\ >nul 2>&1
type build.log
