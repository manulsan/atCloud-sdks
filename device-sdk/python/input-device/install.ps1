# Powershell install script for input-device
Set-StrictMode -Version Latest

Write-Host "Creating virtual environment..."
python -m venv venv

Write-Host "Installing/Upgrading pip and requirements..."
venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r requirements.txt

# Apply local patches if available
$patchFile = Join-Path -Path (Split-Path -Parent $PSScriptRoot) -ChildPath "..\..\..\patches\engineio-async-client-loglevel.patch"
if (Test-Path $patchFile) {
    if (Get-Command git -ErrorAction SilentlyContinue) {
        Write-Host "Applying patch: $patchFile"
        git apply $patchFile -v -p0 || Write-Warning "git apply failed - please apply the patch manually"
    }
    else {
        Write-Warning "git not found - skipping automatic patch application. Apply: $patchFile"
    }
}

Write-Host "\nSetup complete. To activate later: .\venv\Scripts\Activate.ps1 (PowerShell) or venv\Scripts\activate (cmd)"