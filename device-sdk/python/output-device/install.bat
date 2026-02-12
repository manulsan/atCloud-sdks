@echo off
echo Creating virtual environment...
python -m venv venv

echo Installing/Upgrading pip and requirements...
call venv\Scripts\activate
python -m pip install --upgrade pip
pip install -r requirements.txt

echo.
echo Setup complete. To activate later: venv\Scripts\activate (cmd) or .\venv\Scripts\Activate.ps1 (PowerShell)
pause