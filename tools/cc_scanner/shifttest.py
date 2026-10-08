"""Which gear change raises "Sanziman. Dikkatli surun"? One transition per test, each
after an ignition cycle (which clears the warning). Real firmware, fake SimHub.

usage: python shifttest.py OUTDIR
Gear strings as SimHub sends them: "N", "R", "1".."9" (shown S1..), anything else = D.
hand=1 + N + stopped shows P.
"""
import sys, os, time, cv2, numpy as np
from fakesimhub import FakeSimHub, fields

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
# (label, gear A, gear B, speed, hand A)
TESTS = [("N-D", "N", "D", 0, 0), ("D-N", "D", "N", 0, 0), ("N-S1", "N", "1", 0, 0),
         ("S1-S2@30", "1", "2", 30, 0), ("D-N@30", "D", "N", 30, 0), ("S1-N@30", "1", "N", 30, 0)]
cam = cv2.VideoCapture(1, cv2.CAP_DSHOW)
cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920); cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
for _ in range(20): cam.read()
sh = FakeSimHub()

def hold(secs, **kw):
    t = time.time()
    while time.time() - t < secs:
        sh.send(fields(fuel=80, **kw)); time.sleep(0.1)

def mid(label):
    for _ in range(5): cam.read()
    ok, f = cam.read()
    m = cv2.rotate(f, cv2.ROTATE_180)[640:860, 700:1260].copy()
    cv2.putText(m, label, (5, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
    return m

rows = []
for label, a, b, v, ha in TESTS:
    hold(12, ign=0, rpm=0, gear="N", hand=1)                 # ignition cycle clears the CC
    hold(20, ign=1, gear=a, speed=v, hand=ha, rpm=900)   # past the 7 s ignition-on wake
    before = mid(f"{label} before")
    hold(8, ign=1, gear=b, speed=v, hand=0, rpm=900)
    after = mid(f"{label} after")
    rows.append(np.hstack([before, after])); print(label, flush=True)
cv2.imwrite(f"{OUT}/sheet.jpg", np.vstack(rows))
hold(2, ign=1, gear="N", speed=0)
cam.release()
