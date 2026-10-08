#!/bin/bash
# Build + flash each REFUEL_MODE and replay an ETS2 refuel (10 s fill, ignition on, drive off 2 s later)
CLI="/c/Users/fatih/AppData/Local/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe"
cd "$(dirname "$0")/../.."
for m in 0 1 2 3; do
  "$CLI" compile --fqbn arduino:avr:nano --build-property "compiler.cpp.extra_flags=-DREFUEL_MODE=$m" firmware/bmw_f30_cluster 2>&1 | grep -E "error|Global"
  "$CLI" upload --fqbn arduino:avr:nano -p COM30 firmware/bmw_f30_cluster 2>&1 | tail -1
  (cd tools/cc_scanner && rm -rf captures/rmode$m && python -u refueltest.py captures/rmode$m 10 0 1 2 2>&1 | grep -a -v VCAMDS | tr '\n' ' ' && python fuelsheet.py captures/rmode$m 2)
  echo "MODE $m DONE"
done
echo ALLDONE
