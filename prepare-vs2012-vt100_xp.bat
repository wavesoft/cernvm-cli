@echo off
set BUILDTYPE=%1%
if "%BUILDTYPE%"=="" set BUILDTYPE=Release
set BUILDDIR=build_vs2012_t100_xp-%BUILDTYPE%
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"
cd "%BUILDDIR%"
cmake -DCMAKE_BUILD_TYPE=%BUILDTYPE% -DCRASH_REPORTING=ON -DTARGET_ARCH="i386" -G"Visual Studio 11" -T"v110_xp" ..
cd ..
