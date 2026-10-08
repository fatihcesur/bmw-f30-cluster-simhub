"""Shift through gears with the real firmware (fake SimHub) and photograph the MID.

usage: python geartest.py OUTDIR
Each step: (seconds, gear, speed). One MID crop per step end goes to OUTDIR/NN_<gear>.jpg
and all of them into OUTDIR/sheet.jpg, to see which shift raises "Sanziman. Dikkatli surun".
"""
import sys, os, time, cv2, numpy as np
from fakesimhub import FakeSimHub, fields

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
STEPS = [(15, "N", 0), (6, "1", 0), (6, "2", 0), (6, "N", 0), (6, "R", 0), (6, "N", 0),
         (6, "1", 0), (8, "1", 20), (8, "2", 30), (8, "3", 40), (6, "N", 30), (6, "4", 40),
         (8, "N", 0), (6, "D", 0), (6, "N", 0)]
cam = cv2.VideoCapture(1, cv2.CAP_DSHOW)
cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920); cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
for _ in range(20): cam.read()
sh = FakeSimHub()
t = time.time()                                   # ignition cycle first: clears a latched CC
while time.time() - t < 12:
    sh.send(fields(ign=0, rpm=0, gear="N", fuel=80)); time.sleep(0.1)
tiles = []
for i, (secs, g, v) in enumerate(STEPS):
    t = time.time()
    while time.time() - t < secs:
        sh.send(fields(gear=g, speed=v, fuel=80, rpm=900 if v == 0 else 1500)); time.sleep(0.1)
        if time.time() - t > secs - 0.6:
            for _ in range(3): cam.read()
    ok, f = cam.read()
    f = cv2.rotate(f, cv2.ROTATE_180)
    mid = f[640:860, 700:1260].copy()               # MID (full-resolution, upright)
    cv2.putText(mid, f"{i:02d} {g} {v}km/h", (5, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
    cv2.imwrite(f"{OUT}/{i:02d}_{g}.jpg", mid); tiles.append(mid)
    print(i, g, v, flush=True)
while len(tiles) % 4: tiles.append(np.zeros_like(tiles[0]))
cv2.imwrite(f"{OUT}/sheet.jpg", np.vstack([np.hstack(tiles[k:k + 4]) for k in range(0, len(tiles), 4)]))
cam.release()
