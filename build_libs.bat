set TOP=lib

echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\VsDevCmd.bat"
cd %~dp0

:building_driver
echo Building DLLs...
pushd %TOP%\c\ & nmake clean & nmake & nmake DEBUG & popd > nul
pushd "%TOP%\c++\" & nmake clean & nmake & nmake DEBUG & popd > nul
pushd %TOP%\net\ & nmake clean & nmake & nmake DEBUG & popd > nul

exit