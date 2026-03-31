#!/usr/bin/bash
set -euo pipefail


echo "Restarting the app"
systemctl --user restart music.service
