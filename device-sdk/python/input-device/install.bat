@echo off
echo Creating virtual environment...
python -m venv venv

IF NOT DEFINED ENGINEIO_LOG_LEVEL (
  set ENGINEIO_LOG_LEVEL=WARNING
)
echo ENGINEIO_LOG_LEVEL=%ENGINEIO_LOG_LEVEL% (default WARNING)

echo Installing/Upgrading pip and requirements...
call venv\Scripts\activate
python -m pip install --upgrade pip
pip install -r requirements.txt

REM Apply local patches if available
SET PATCH_FILE=%~dp0\..\..\..\patches\engineio-async-client-loglevel.patch
IF EXIST "%PATCH_FILE%" (
  where git >nul 2>&1 && (
    echo Applying patch: %PATCH_FILE%
    git apply "%PATCH_FILE%" || echo git apply failed - please apply the patch manually
  ) || (
    echo git not found - skipping automatic patch application. Apply: %PATCH_FILE%
  )
)

echo.
echo Setup complete. To activate later: venv\Scripts\activate (cmd) or .\venv\Scripts\Activate.ps1 (PowerShell)
pause