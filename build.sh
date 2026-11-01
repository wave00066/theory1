#!/bin/bash
# THEORY-1's firmware: base/tools/build.py app [--release X.Y] -> app/build/theory1.fwsc
# Needs JieLi's toolchain ($JIELI_TOOLCHAIN, base/tools/get_toolchain.sh) and the AC79 SDK ($AC79_SDK) for
# Bluetooth; the GitHub workflow (.github/workflows/release.yml) has both.
set -euo pipefail
cd "$(dirname "$0")"
python3 base/tools/build.py app "$@"
