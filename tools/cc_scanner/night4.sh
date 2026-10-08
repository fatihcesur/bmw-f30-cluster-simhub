#!/bin/sh
# 0x2C4 bytes at a fake 80 km/h / 2000 rpm, looking for what moves the consumption needle.
cd "$(dirname "$0")"
for i in 1 2 3 4 5 6; do python sweep.py c2c4$i 80 16 || echo "ERROR sweep c2c4$i"; done
echo "NIGHT4 DONE"
