#!/bin/sh
# Cruise bytes again at a fake 80 km/h / 2000 rpm with a clean MID.
cd "$(dirname "$0")"
for i in 1 2 3 5 6; do python sweep.py cruise$i 80 8 || echo "ERROR sweep cruise$i"; done
echo "NIGHT3 DONE"
