#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Convert Chrome-trace .dms files to Vampir-compatible OTF2 (traces.otf2).
# Usage: convert_dms_to_otf2.sh [path/to/file.dms ...]
#        With no args, converts every *.dms under stage{2,3,4,5}*/results/
#        that is missing a sibling <stem>/traces.otf2 (OTF2 required for all stages).

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TC_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${TC_ROOT}/../.." && pwd)}"

OTF2_PY="${ROOT}/.venv-discostic/bin/python"
if [ ! -x "${OTF2_PY}" ]; then
    OTF2_PY="$(command -v python3 || command -v python)"
fi
if ! "${OTF2_PY}" -c "import otf2" >/dev/null 2>&1; then
    echo "ERROR: otf2 package missing (tried ${OTF2_PY})" >&2
    exit 1
fi

convert_one() {
    local dms=$1
    local base dir out
    dir="$(cd "$(dirname "${dms}")" && pwd)"
    base="$(basename "${dms}" .dms)"
    out="${dir}/${base}"
    if [ -f "${out}/traces.otf2" ] && [ "$(wc -c < "${out}/traces.otf2" | tr -d ' ')" -ge 50 ]; then
        echo "SKIP (OTF2 exists): ${out}/traces.otf2"
        return 0
    fi
    echo "===== ${dms} → ${out}/traces.otf2 ====="
    rm -rf "${out}"
    (
        cd "${dir}"
        "${OTF2_PY}" "${SCRIPT_DIR}/chrome2otf2.py" -i "${base}.dms" -o "${base}" -c
    ) 2>&1 | grep -v 'Ignoring unknown event key' || true
    test -f "${out}/traces.otf2"
    echo "OK: ${out}/traces.otf2 ($(du -h "${out}/traces.otf2" | awk '{print $1}'))"
}

if [ "$#" -gt 0 ]; then
    for dms in "$@"; do
        convert_one "${dms}"
    done
    exit 0
fi

mapfile -t DMS_FILES < <(
    find \
        "${TC_ROOT}/stage2-single-timestep/results" \
        "${TC_ROOT}/stage3-multi-timestep/results" \
        "${TC_ROOT}/stage4-idlewave/results" \
        "${TC_ROOT}/stage5-scaling/results" \
        -type f -name '*.dms' 2>/dev/null | sort
)
if [ "${#DMS_FILES[@]}" -eq 0 ]; then
    echo "No .dms files found under Stage 2–5 results."
    exit 0
fi
# Smallest first so Stage 2/3 complete before multi-GB Stage 5 traces.
mapfile -t DMS_FILES < <(ls -1SSr "${DMS_FILES[@]}" 2>/dev/null || printf '%s\n' "${DMS_FILES[@]}")
for dms in "${DMS_FILES[@]}"; do
    convert_one "${dms}"
done
echo "Processed ${#DMS_FILES[@]} DMS file(s)."
