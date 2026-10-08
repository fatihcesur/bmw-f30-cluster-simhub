#!/bin/bash
# Repeat consistent drive+refuel with the main firmware, read 0x330 with the scanner after each.
CLI="/c/Users/fatih/AppData/Local/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe"
cd "$(dirname "$0")"
for i in 1 2 3 4 5; do
  "$CLI" upload --fqbn arduino:avr:nano -p COM30 ../../firmware/bmw_f30_cluster 2>&1 | grep -q "New upload port" || { echo "ERROR upload main $i"; exit 1; }
  timeout 400 python -u consistent.py captures/unlearn$i 2>&1 | grep -a -v VCAMDS | tr '\n' ' '; echo
  "$CLI" upload --fqbn arduino:avr:nano -p COM30 cc_scanner 2>&1 | grep -q "New upload port" || { echo "ERROR upload scanner $i"; exit 1; }
  echo "ROUND $i: $(timeout 90 python -u read330.py 99 45 2>&1 | grep -a litres | tail -1)"
done
"$CLI" upload --fqbn arduino:avr:nano -p COM30 ../../firmware/bmw_f30_cluster 2>&1 | tail -1
echo ALLDONE
