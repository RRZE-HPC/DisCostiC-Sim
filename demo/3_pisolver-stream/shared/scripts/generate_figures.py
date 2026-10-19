#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Redirect to demo-root generate_figures.py (pisolver-stream-demo owns its figures).

from pathlib import Path
import runpy

runpy.run_path(str(Path(__file__).resolve().parents[2] / "generate_figures.py"), run_name="__main__")
