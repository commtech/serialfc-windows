set NAME=serialfc
set TOP=bin\%NAME%

echo off

:reset_bin_folder
echo Removing Old Drivers...
rmdir /S /Q bin\ 2> nul
mkdir %TOP%\
rmdir /S /Q tmp\production\ 2> nul
mkdir tmp\production\

:build_drivers
echo Building Release Drivers...
start /I /WAIT build_driver.bat "fre x86 WXP" objfre_wxp_x86 i386 production XP_X86
start /I /WAIT build_driver.bat "fre x64 WNET" objfre_wnet_amd64 amd64 production Server2003_X64

:build_libs
echo Building Libraries...
start /I /WAIT build_libs.bat

:create_directories
echo Creating Directories...
for %%A in (32, 64, lib) do mkdir %TOP%\%%A\
for %%A in (c, c++, net, python) do mkdir %TOP%\lib\%%A\

:copy_dll_files
echo Copying DLL Files...
copy lib\c\cserialfc*.dll %TOP%\lib\c\ > nul
copy lib\c\cserialfc*.lib %TOP%\lib\c\ > nul
copy lib\c\src\*.c %TOP%\lib\c\ > nul
copy lib\c\src\*.h %TOP%\lib\c\ > nul
copy lib\c\makefile %TOP%\lib\c\ > nul
copy "lib\c++\cppserialfc*.dll" "%TOP%\lib\c++\" > nul
copy "lib\c++\cppserialfc*.lib" "%TOP%\lib\c++\" > nul
copy lib\c\cserialfc*.dll "%TOP%\lib\c++\" > nul
copy "lib\c++\src\*.cpp" "%TOP%\lib\c++\" > nul
copy "lib\c++\src\*.hpp" "%TOP%\lib\c++\" > nul
copy "lib\c++\makefile" "%TOP%\lib\c++\" > nul
copy lib\net\netserialfc*.dll %TOP%\lib\net\ > nul
copy lib\c\cserialfc*.dll %TOP%\lib\net\ > nul
copy lib\net\src\*.cs %TOP%\lib\net\ > nul
copy lib\net\makefile %TOP%\lib\net\ > nul
copy lib\python\serialfc.py %TOP%\lib\python\ > nul

:copy_sys_files
echo Copying Driver Files...
copy tmp\production\i386\* %TOP%\32\ > nul
copy tmp\production\amd64\* %TOP%\64\ > nul

:copy_setup_files
:echo Copying Setup Files...
:copy redist\production\i386\dpinst.exe %TOP%\32\setup.exe > nul
:copy redist\production\amd64\dpinst.exe %TOP%\64\setup.exe > nul
:copy_changelog
echo Copying Changelog...
copy ChangeLog.txt %TOP% > nul

:zip_packages
echo Zipping drivers...
cd %TOP%\ > nul
cd ..\ > nul
..\7za.exe a -tzip %NAME%.zip %NAME%\ > nul
cd ..\ > nul
