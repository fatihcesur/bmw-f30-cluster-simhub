"""Contact sheet of the fuel gauge from a refueltest.py run: python fuelsheet.py DIR STEP"""
import sys, glob, os, cv2, numpy as np
d, step = sys.argv[1], int(sys.argv[2]) if len(sys.argv) > 2 else 3
fs = sorted(glob.glob(f"{d}/[0-9]*.jpg"))[::step]
tiles = []
for p in fs:
    f = cv2.imread(p)[430:710, 30:350]     # fuel gauge; camera upright since 2026-10-04 evening
    name = os.path.basename(p)[:-4]
    cv2.putText(f, name, (5, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 1)
    tiles.append(f)
cols = 10
while len(tiles) % cols: tiles.append(np.zeros_like(tiles[0]))
rows = [np.hstack(tiles[i:i + cols]) for i in range(0, len(tiles), cols)]
cv2.imwrite(f"{d}/sheet.jpg", np.vstack(rows))
