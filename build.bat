
@echo off
setlocal
set "MSYS2_PATH=C:\msys64\usr\bin;C:\msys64\ucrt64\bin"
set "PATH=%MSYS2_PATH%;%PATH%"
make deps PLAT=WINDOWS > build.log 2>&1 | tail -f build.log

@REM PROXY = https://gh-proxy.org/
@REM REPO_OPENGL  ?= $(PROXY)https://github.com/KhronosGroup/OpenGL-Registry.git
@REM REPO_EGL     ?= $(PROXY)https://github.com/KhronosGroup/EGL-Registry.git
@REM REPO_GLFIXES ?= $(PROXY)https://github.com/nigels-com/glfixes
