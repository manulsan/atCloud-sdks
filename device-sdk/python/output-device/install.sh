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

# Default ENGINEIO log level for this install (can be overridden by env)
export ENGINEIO_LOG_LEVEL=${ENGINEIO_LOG_LEVEL:-WARNING}
echo "ENGINEIO_LOG_LEVEL=${ENGINEIO_LOG_LEVEL} (default WARNING)"

echo "Upgrading pip and installing requirements..."
python -m pip install --upgrade pip
pip install -r requirements.txt

# Apply local patches if available
PATCH_FILE="$(dirname "$0")/../../../patches/engineio-async-client-loglevel.patch"
if [ -f "$PATCH_FILE" ]; then
  if command -v git >/dev/null 2>&1; then
    echo "Applying patch: $PATCH_FILE"
    git apply "$PATCH_FILE" || echo "git apply failed - please apply the patch manually"
  else
    echo "git not found - skipping automatic patch application. Apply: $PATCH_FILE"
  fi
fi

cat <<'EOF'

Setup complete.
To use the virtual environment:
 - Linux/macOS:  source venv/bin/activate
 - Windows PowerShell:  .\venv\Scripts\Activate.ps1
 - Windows cmd:  venv\Scripts\activate

EOF