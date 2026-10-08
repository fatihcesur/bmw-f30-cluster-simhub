"""Hunt for the speed-limit-info (SLI) sign: send a periodic frame on every unknown CAN id
and look for any change on the cluster with the camera.

usage: python slihunt.py PATTERN START END [speed] [counter_byte]
  PATTERN: 8 hex bytes joined by '-', e.g. 50-50-50-50-50-50-50-50 (80 km/h everywhere);
           several patterns separated by ',' are tried one after another on every id
  START/END: hex id range, e.g. 0A0 4FF
Per id: reference frame -> "P id bytes" for ON_S -> frame -> "P -" -> wait until the
cluster looks like the reference again. Hits go to captures/sli/<pattern>/ (ref | on |
changed pixels) and print "HIT <id> <pixels>". Every id prints one progress line.
The camera hangs upside down: frames are rotated 180 degrees.
"""
import sys, os, time, csv, cv2, numpy as np
from scanlib import Rig

PATS = [p.replace("-", " ") for p in sys.argv[1].split(",")]
START, END = int(sys.argv[2], 16), int(sys.argv[3], 16)
SPEED = int(sys.argv[4]) if len(sys.argv) > 4 else 0
CNT = sys.argv[5] if len(sys.argv) > 5 else "-"   # byte whose low nibble is an alive counter
ON_S = 2.0
THRESH = 50                     # edge pixels in the largest new blob to call it a hit

# ids we already send (real firmware / scanner) and ids the cluster transmits itself
SKIP = {0x0D7, 0x0F3, 0x12F, 0x193, 0x19B, 0x1A1, 0x1EE, 0x1F6, 0x202, 0x21A, 0x289,
        0x291, 0x297, 0x2A7, 0x2BB, 0x2C3, 0x2C4, 0x349, 0x34F, 0x368, 0x36E, 0x39E,
        0x3A7, 0x3F9, 0x3FD, 0x5C0,
        0x1B3, 0x205, 0x2C5, 0x2CA, 0x2F7, 0x328, 0x330, 0x336, 0x338, 0x35C, 0x362,
        0x367, 0x393, 0x560, 0x5E0, 0x660}

OUT = f"captures/sli/{sys.argv[1].replace(',', '+')}_s{SPEED}" + ("" if CNT == "-" else f"_c{CNT}")
os.makedirs(OUT, exist_ok=True)

def small(f):
    f = cv2.rotate(f, cv2.ROTATE_180)
    return cv2.resize(f, None, fx=0.5, fy=0.5)

# chrome bezel rings reflect the room light (clouds/sun) - built from false hits
RING = cv2.imread("ring_mask.png", 0) if os.path.exists("ring_mask.png") else None

# only look where a sign/icon can appear: the MID and the inner dial faces
# (half-resolution coordinates, camera framing of 2026-10-04)
ROI = np.zeros((540, 960), np.uint8)
cv2.rectangle(ROI, (365, 340), (620, 425), 255, -1)       # MID (panel only, not the glass above it)
cv2.circle(ROI, (290, 228), 125, 255, -1)                 # speedometer face
# tachometer face left out: the needle wobbles with the fake rpm
cv2.rectangle(ROI, (420, 240), (530, 330), 255, -1)       # warning lamps between the dials

cv2.circle(ROI, (300, 228), 32, 0, -1)                    # needle hubs: glare changes with
cv2.circle(ROI, (665, 213), 32, 0, -1)                    # the cluster's auto brightness
cv2.rectangle(ROI, (585, 245), (760, 340), 0, -1)       # small l/100km gauge: needle moves while "driving"
ROI[285:362, 210:460] = 0        # cover-glass reflection between the dials (room light moves)
ROI[368:400, 365:445] = 0        # MID clock digits (a CC triangle appears just above)
ROI[355:400, 455:600] = 0        # MID average consumption keeps changing
ROI[385:420, 330:600] = 0        # odometer / trip roll over while "driving"

def norm(f):
    """Grey, scaled so the brightest ROI pixels match: the cluster dims its own
    backlight with the room light, which must not count as a change."""
    g = cv2.GaussianBlur(cv2.cvtColor(f, cv2.COLOR_BGR2GRAY), (3, 3), 0).astype(np.float32)
    p = np.percentile(g[ROI > 0], 99)
    return np.clip(g * (200.0 / max(p, 1)), 0, 255).astype(np.uint8)

def diff(a, b):
    """Count edges in b that are not near any edge of a, inside the ROI: a new icon,
    sign or text adds edges, a brightness change does not."""
    ea, eb = cv2.Canny(norm(a), 60, 150) > 0, cv2.Canny(norm(b), 60, 150) > 0
    near = cv2.dilate(ea.astype(np.uint8), np.ones((7, 7), np.uint8)) > 0
    m = eb & ~near & (ROI > 0)
    # score = edge pixels of the largest compact cluster: an icon or sign is one blob,
    # glass reflections and needle glints are scattered specks
    n, lab, _, _ = cv2.connectedComponentsWithStats(cv2.dilate(m.astype(np.uint8), np.ones((9, 9), np.uint8)))
    best = max((int(m[lab == i].sum()) for i in range(1, n)), default=0)
    return best, m

r = Rig(cam=1)
for c in ("v ign 1", "v light 1", "v fuel 80", f"v speed {SPEED}", "P -", f"Q {CNT}"):
    r.cmd(c)
time.sleep(10)
log = csv.writer(open(f"{OUT}/log.csv", "a", newline=""))
try:
    for cid in range(START, END + 1):
        if cid in SKIP:
            continue
        score = 0
        for k, pat in enumerate(PATS):
            ref = small(r.frame())
            r.cmd(f"P {cid:X} {pat}")
            time.sleep(ON_S)
            on = small(r.frame())
            r.cmd("P -")
            sc, m = diff(ref, on)
            score = max(score, sc)
            hit = sc >= THRESH
            if hit:
                marked = on.copy(); marked[m] = (0, 255, 255)
                cv2.imwrite(f"{OUT}/{cid:03X}_{k}.jpg", np.hstack([ref, on, marked]))
                print(f"HIT {cid:03X} pattern{k} {sc}", flush=True)
                t0 = time.time()                 # let it recover before the next try
                while time.time() - t0 < 20 and diff(ref, small(r.frame()))[0] >= THRESH:
                    time.sleep(1)
            log.writerow([f"{cid:03X}", k, sc, int(hit), time.strftime("%H:%M:%S")])
        print(f"id {cid:03X} {score}", flush=True)
    print("DONE", flush=True)
except Exception as e:
    print("ERROR", type(e).__name__, e, flush=True)
    raise
finally:
    try: r.cmd("P -"); r.cmd("v speed 0")
    except Exception: pass
    r.close()
