#!/usr/bin/bash
set -euo pipefail

./scripts/build_release.sh

./scripts/restart.sh
