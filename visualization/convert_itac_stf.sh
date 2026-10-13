#!/bin/bash
# Convert a merged DisCostiC .avt event log to Intel STF (ITAC).
# Usage: convert_itac_stf.sh input.avt output.single.stf
set -euo pipefail
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 input.avt output.single.stf" >&2
    exit 2
fi
IN=$1
OUT=$2
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

if ! type module >/dev/null 2>&1; then
    if [ -f /usr/share/Modules/init/bash ]; then
        # shellcheck disable=SC1091
        . /usr/share/Modules/init/bash
    elif [ -f /etc/profile.d/modules.sh ]; then
        # shellcheck disable=SC1091
        . /etc/profile.d/modules.sh
    fi
fi
module load intel-itac/2022.4.0 2>/dev/null || module load intel-itac 2>/dev/null || true

PY="${ROOT}/.venv-discostic/bin/python"
if [ ! -x "${PY}" ]; then
    PY="$(command -v python3 || command -v python)"
fi
if ! "${PY}" -c "import otf2" >/dev/null 2>&1; then
    echo "Installing otf2 Python package into the converter interpreter..." >&2
    "${PY}" -m pip install --quiet otf2
fi
if ! command -v otf2-to-stf >/dev/null 2>&1; then
    echo "ERROR: otf2-to-stf not on PATH (load intel-itac)" >&2
    exit 1
fi
"${PY}" "${SCRIPT_DIR}/events2stf.py" -i "${IN}" -o "${OUT}"
