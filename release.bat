set NAME=serialfc-windows-2.4.5
set TOP=bin\%NAME%
set QSERIALFC=..\qserialfc\build\exe.win32-3.3
set PYSERIALFC=..\pyserialfc\dist

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
if %errorlevel% neq 0 exit /b %errorlevel%
start /I /WAIT build_driver.bat "fre x64 WNET" objfre_wnet_amd64 amd64 production Server2003_X64
if %errorlevel% neq 0 exit /b %errorlevel%

:build_libs
echo Building Libraries...
start /I /WAIT build_libs.bat
if %errorlevel% neq 0 exit /b %errorlevel%

:create_directories
echo Creating Directories...
for %%A in (32, 64, lib, terminal, gui, test) do mkdir %TOP%\%%A\
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

:copy_test_files
echo Copying Test Files...
copy lib\c\utils\test\test.exe %TOP%\test\ > nul
copy lib\c\cserialfc.dll %TOP%\test\ > nul

:copy_sys_files
echo Copying Driver Files...
copy tmp\production\i386\* %TOP%\32\ > nul
copy tmp\production\amd64\* %TOP%\64\ > nul

:copy_python_files
echo Copying Python Files...
copy %PYSERIALFC%\pyserialfc*.exe* %TOP%\lib\python\ > nul

:copy_terminal_files
echo Copying Terminal Files...
xcopy redist\production\terminal\* %TOP%\terminal\ /e /i > nul

:copy_gui_files
echo Copying GUI Files...
xcopy %QSERIALFC%\* %TOP%\gui\ /e /i > nul

:copy_changelog
echo Copying Changelog...
copy ChangeLog.txt %TOP% > nul

:copy_readme
echo Copying README...
copy README.md %TOP% > nul

:zip_packages
echo Zipping Drivers...
cd %TOP%\ > nul
cd ..\ > nul
..\7za.exe a -tzip %NAME%.zip %NAME%\ > nul
cd ..\ > nul
