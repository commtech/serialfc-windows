set NAME=serialfc-windows-3.0.0.0
set TOP=release\%NAME%
set QSERIALFC=..\qserialfc\build\exe.win32-3.3
set PYSERIALFC=..\pyserialfc\dist

echo off

:reset_bin_folder
echo Removing Old Drivers...
rmdir /S /Q release\ 2>NUL
mkdir %TOP%\ >NUL

:create_directories
echo Creating Directories...
for %%A in (64, lib, terminal, gui, examples, docs) do mkdir %TOP%\%%A\ >NUL
for %%A in (raw) do mkdir %TOP%\lib\%%A\ >NUL

:copy_dll_files
echo Copying DLL Files...
copy lib\raw\*.h %TOP%\lib\raw\ >NUL
copy lib\raw\*.c %TOP%\lib\raw\ >NUL

:copy_example_files
echo Copying Example Files...
xcopy examples\*.c %TOP%\examples\ /s /i >NUL

:copy_tool_files
echo Copying Example Files...
xcopy tools\*.c %TOP%\tools\ /s /i >NUL

:copy_docs_files
echo Copying Docs Files...
xcopy docs\*.md %TOP%\docs\ /e /i >NUL

:copy_readme
echo Copying README...
copy README.md %TOP% >NUL

:copy_changelog
echo Copying Changelog...
copy ChangeLog.md %TOP% >NUL

:copy_license
echo Copying License...
copy LICENSE.md %TOP% >NUL

:generate_cab
echo Generating .cab files for Windows 10..
makecab /f serialfc_64.ddf >NUL
copy tmp\64\serialfc.cab %TOP%\64 >NUL


:sign_cab
signtool sign /fd SHA512 /n "Commtech, Inc." /t http://timestamp.digicert.com/ /sha1 B757F8701A8CE35E8A243A12207CE8A697756DF0 %TOP%\64\serialfc.cab
if %errorlevel% neq 0 exit /b %errorlevel%



:build_libs
:echo Building Libraries...
:start /I /WAIT build_libs.bat
:if %errorlevel% neq 0 exit /b %errorlevel%

:copy_terminal_files
:echo Copying Terminal Files...
:xcopy redist\production\terminal\* %TOP%\terminal\ /e /i > nul

:copy_gui_files
:echo Copying GUI Files...
:xcopy %QSERIALFC%\* %TOP%\gui\ /e /i > nul
