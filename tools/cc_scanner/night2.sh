#!/bin/sh
# Second overnight phase: redo codes 0-49 with the rolling reference, then the byte sweeps.
cd "$(dirname "$0")"
python scan.py 0 49 || echo "ERROR scan 0-49"
for m in lights0 lights1 back1 cruise1 cruise2 cruise3 cruise4 cruise5 cruise6; do
  python sweep.py $m || echo "ERROR sweep $m"
done
echo "NIGHT2 DONE"
