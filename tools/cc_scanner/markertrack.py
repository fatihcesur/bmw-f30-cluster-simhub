"""Track the cruise set-speed marker LED while sweeping 0x289 bytes.

0x289 byte1 = 0x20 lights the marker (orange, found by cruisehunt.py on 2026-10-09).
With that held, sweep another byte and log where the LED sits on the speedometer ring
(angle, as a km/h estimate) and its colour.

usage: python markertrack.py [BYTES] [STEP] [BASE]
  BYTES  comma list of 0x289 byte indexes to sweep (default 2,3,4,5,6)
  STEP   value step (default 4)
  BASE   byte1 value that lights the marker (hex, default 20)
Output: captures/cruise/track_b<idx>.csv and a contact strip track_b<idx>.jpg; lines
"b<idx> <val> <kmh> <colour>" and "MOVED"/"COLOUR" when something changes.
"""
import sys, os, time, csv, math, cv2, numpy as np
from scanlib import Rig

CX, CY, R_IN, R_OUT = 564, 690, 200, 292      # speedometer scale ring (Brio, framing 2026-10-09)
OUT = "captures/cruise"
BYTES = [int(b) for b in (sys.argv[1] if len(sys.argv) > 1 else "2,3,4,5,6").split(",")]
STEP = int(sys.argv[2]) if len(sys.argv) > 2 else 4
BASE = sys.argv[3] if len(sys.argv) > 3 else "20"

ROI = np.zeros((1080, 1920), np.uint8)
cv2.circle(ROI, (CX, CY), R_OUT, 255, -1)
cv2.circle(ROI, (CX, CY), R_IN, 0, -1)
cv2.circle(ROI, (340, 894), 45, 0, -1)        # green lights-on icon below the dial

# dial calibration from the camera frame: angle of the 0 and 260 marks (image coords,
# degrees, measured from +x clockwise because y points down)
A0, A260 = 151.0, 30.0 + 360.0               # 0 km/h lower left, 260 lower right (frame 2026-10-09)

def find(f):
    h = cv2.cvtColor(cv2.GaussianBlur(f, (5, 5), 0), cv2.COLOR_BGR2HSV)
    H, S, V = h[..., 0], h[..., 1], h[..., 2]
    g = (H >= 40) & (H <= 90) & (S >= 90) & (V >= 90) & (ROI > 0)
    o = (H >= 4) & (H <= 22) & (S >= 140) & (V >= 120) & (ROI > 0)
    m = (g | o).astype(np.uint8)
    n, lab, st, cen = cv2.connectedComponentsWithStats(m)
    if n <= 1:
        return None
    i = 1 + int(np.argmax(st[1:, cv2.CC_STAT_AREA]))
    if st[i, cv2.CC_STAT_AREA] < 15:
        return None
    x, y = cen[i]
    ang = math.degrees(math.atan2(y - CY, x - CX)) % 360
    if ang < A0:
        ang += 360
    kmh = (ang - A0) / (A260 - A0) * 260
    col = "green" if (g & (lab == i)).sum() >= (o & (lab == i)).sum() else "orange"
    return round(kmh), col, (int(x), int(y))

r = Rig(cam=0)
for c in ("v ign 1", "v light 1", "v fuel 80", "v rpm 2000", "v speed 80", "v cruise 1", "P -", "Q -", "o -"):
    r.cmd(c)
time.sleep(12)
r.cmd(f"o 289 1 {BASE}")
time.sleep(3)
print("start", find(r.frame()), flush=True)
try:
    for idx in BYTES:
        w = csv.writer(open(f"{OUT}/track_b{idx}.csv", "w", newline=""))
        strip, prev = [], None
        for val in range(0, 256, STEP):
            r.cmd(f"o 289 {idx} {val:X}")
            time.sleep(1.5)
            f = r.frame()
            res = find(f)
            kmh, col = (res[0], res[1]) if res else (None, "none")
            w.writerow([idx, val, kmh, col])
            print(f"b{idx} {val:02X} {kmh} {col}", flush=True)
            if prev is not None and (col != prev[1] or (kmh is not None and prev[0] is not None and abs(kmh - prev[0]) >= 8)):
                print(f"{'COLOUR' if col != prev[1] else 'MOVED'} b{idx} {val:02X}: {prev} -> {(kmh, col)}", flush=True)
                crop = f[CY - R_OUT:CY + R_OUT, CX - R_OUT:CX + R_OUT].copy()
                cv2.putText(crop, f"b{idx}={val:02X} {kmh} {col}", (10, 30), 0, 1, (0, 255, 0), 2)
                strip.append(cv2.resize(crop, (300, 300)))
            prev = (kmh, col)
        if strip:
            rows = [np.hstack(strip[i:i + 6] + [np.zeros_like(strip[0])] * (6 - len(strip[i:i + 6]))) for i in range(0, len(strip), 6)]
            cv2.imwrite(f"{OUT}/track_b{idx}.jpg", np.vstack(rows))
        r.cmd(f"o -"); r.cmd(f"o 289 1 {BASE}"); time.sleep(2)
    print("DONE", flush=True)
except Exception as e:
    print("ERROR", type(e).__name__, e, flush=True)
    raise
finally:
    try:
        r.cmd("o -"); r.cmd("v speed 0"); time.sleep(2)
    except Exception:
        pass
    r.close()
