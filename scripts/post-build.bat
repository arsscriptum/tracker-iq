@echo off
REM post-build.bat - Copies a variable number of files to the destination folder.
REM Usage: post-build.bat "destination_folder" "file1" "file2" "file3" ...

REM Check if at least two arguments are provided.
if "%~1"=="" (
    echo [ERROR] Destination folder is missing.
    goto :Usage
)
if "%~2"=="" (
    echo [ERROR] At least one file path must be provided.
    goto :Usage
)

echo "=================================================================="
echo "                 starting post-build operations                   "
echo "=================================================================="
REM Set destination folder variable.
set "DEST=%~1"

REM Create destination folder if it does not exist.
if not exist "%DEST%" (
    echo Creating destination folder: %DEST%
    mkdir "%DEST%"
)

REM Shift to remove the destination folder from arguments.
shift

REM Process each file argument.
:CopyLoop
if "%~1"=="" goto :End
copy "%~1" "%DEST%" >nul
if errorlevel 1 (
    echo [ERROR] Copy error "%~1" to "%DEST%"
) else (
    echo [SUCCESS] Copied "%~1"
)
shift
goto :CopyLoop

:End
echo All files processed.
echo "=================================================================="
echo "               terminating post-build operations                  "
echo "=================================================================="
exit /b 0

:Usage
echo Usage: post-build.bat "destination_folder" "file1" [file2 ...]
exit /b 1
