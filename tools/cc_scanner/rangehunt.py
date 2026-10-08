"""Hunt for the signal that lets the cluster update its range while driving.

Fake 80 km/h drive; for each candidate byte of frames we already send, try each
single-bit flip of its default plus 0x00 / 0xFF for HOLD seconds and watch the
MID range digits with the camera. The displayed range is stale (it was computed at
the last key-on), so a signal that enables live recompute should change it at once.
Hits (digits changed) are saved to captures/rangehunt/ with before/after images.
The range page must be showing on the MID.
"""
import time, os, csv, cv2, numpy as np
from scanlib import Rig

HOLD = 6
RANGE_BOX = (slice(648, 695), slice(940, 1055))     # MID range digits, full frame
MID_BOX = (slice(600, 745), slice(740, 1090))       # whole MID, for the photos
os.makedirs("captures/rangehunt", exist_ok=True)

# (override id, byte index, default value, label). Indices are into the payload the
# firmware passes to applyOv (without the CRC byte for CRC'd frames).
CANDIDATES = (
    [(0x12F, i, d, f"12F b{i + 1}") for i, d in [(1, 0x8A), (2, 0xDD), (3, 0xF1), (4, 0x01), (5, 0x30), (6, 0x06)]]
    + [(0xF3, i, d, f"F3 b{i}") for i, d in [(3, 0xC0), (4, 0xF0), (5, 0x00), (6, 0xFF), (7, 0xFF)]]
    + [(0x3F9, i, 0x00, f"3F9 b{i}") for i in (0, 1, 2, 3, 4, 6, 7)]
    + [(0x2C4, i, d, f"2C4 b{i + 1}") for i, d in [(1, 0xFF), (2, 0x64), (3, 0x64), (4, 0x64), (5, 0x01), (6, 0xF1)]]
)

r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v speed 0", "v rpm 0", "v ign 1", "v fuel 82",
          "v cons 29", "v fk 114", "o -"):
    r.cmd(c)

def grab():
    for _ in range(4): r.cam.read()
    return r.cam.read()[1]

def gray(f):
    return cv2.GaussianBlur(cv2.cvtColor(f[RANGE_BOX], cv2.COLOR_BGR2GRAY), (3, 3), 0)

def changed(a, b):
    return int((cv2.absdiff(gray(a), gray(b)) > 50).sum())

time.sleep(8)
r.cmd("v speed 80"); r.cmd("v rpm 1500"); time.sleep(10)
log = csv.writer(open("captures/rangehunt/log.csv", "w", newline=""))
hits = 0
try:
    for oid, idx, dflt, label in CANDIDATES:
        values = sorted({dflt ^ (1 << b) for b in range(8)} | {0x00, 0xFF})
        for v in values:
            base = grab()
            r.cmd(f"o {oid:x} {idx} {v:x}")
            time.sleep(HOLD)
            now = grab()
            score = changed(base, now)
            r.cmd("o -"); time.sleep(1.5)
            log.writerow([label, f"{v:02X}", score])
            if score > 120:
                hits += 1
                cv2.imwrite(f"captures/rangehunt/{label.replace(' ', '_')}_{v:02X}.jpg",
                            np.hstack([base[MID_BOX], now[MID_BOX]]))
                print(f"HIT {label} = {v:02X}  score {score}", flush=True)
        print(f"done {label}", flush=True)
finally:
    r.cmd("o -"); r.cmd("v speed 0"); r.cmd("v rpm 0"); time.sleep(3)
    r.close()
    print(f"FINISHED, {hits} hits", flush=True)
