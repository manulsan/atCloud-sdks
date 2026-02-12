#!/usr/bin/env bash
set -e

echo "Creating virtual environment..."
python -m venv venv

# Activate if possible (this script will not affect parent shell)
if [ -f "venv/bin/activate" ]; then
  . venv/bin/activate
elif [ -f "venv/Scripts/activate" ]; then
  . venv/Scripts/activate
fi

echo "Upgrading pip and installing requirements..."
python -m pip install --upgrade pip
pip install -r requirements.txt

cat <<'EOF'

Setup complete.
To use the virtual environment:
 - Linux/macOS:  source venv/bin/activate
 - Windows PowerShell:  .\venv\Scripts\Activate.ps1
 - Windows cmd:  venv\Scripts\activate

EOF