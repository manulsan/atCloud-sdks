# Powershell install script for input-device
Set-StrictMode -Version Latest

Write-Host "Creating virtual environment..."
python -m venv venv

Write-Host "Installing/Upgrading pip and requirements..."
venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r requirements.txt

Write-Host "\nSetup complete. To activate later: .\venv\Scripts\Activate.ps1 (PowerShell) or venv\Scripts\activate (cmd)"