"""Look for ANY new light on the cluster (lamps, dial LEDs like "LIM", unknown icons).

Brio (cam 0, head-on). Reference frame vs test frame: new bright blobs that were not there
(after dilation) count. Masked out: the needles (a wedge around each needle), the MID
(clock / consumption keep changing) and the cruise set-speed marker ring (known).

usage: python lamphunt.py b289            sweep 0x289 bytes 1..3 over 0..255 (cruise on, 80 km/h)
       python lamphunt.py ids START END   periodic frames on every other id (3 patterns)
Hits: captures/lamps/<name>.jpg (ref | on | marked), lines "HIT ...", "id ...", "DONE".
Needs the cc_scanner firmware.
"""
import sys, os, time, math, csv, cv2, numpy as np
from scanlib import Rig

OUT = "captures/lamps"
os.makedirs(OUT, exist_ok=True)
SP, TA = (564, 710), (1232, 725)            # needle hubs (Brio framing 2026-10-09)
MIN_BLOB = 60
ON_S = 1.6
SKIP = {0x0D7, 0x0F3, 0x12F, 0x193, 0x19B, 0x1A1, 0x1EE, 0x1F6, 0x202, 0x21A, 0x289,
        0x291, 0x297, 0x2A7, 0x2BB, 0x2C3, 0x2C4, 0x349, 0x34F, 0x368, 0x36E, 0x39E,
        0x3A7, 0x3F9, 0x3FD, 0x5C0,
        0x1B3, 0x205, 0x2C5, 0x2CA, 0x2F7, 0x328, 0x330, 0x336, 0x338, 0x35C, 0x362,
        0x367, 0x393, 0x560, 0x5E0, 0x660}
PATS = [("50 50 50 50 50 50 50 50", "-"), ("00 F0 50 50 50 50 50 50", "1"), ("FF FF FF FF FF FF FF FF", "-")]

BASE = np.zeros((1080, 1920), np.uint8)
cv2.rectangle(BASE, (80, 300), (1880, 1040), 255, -1)            # the cluster
cv2.rectangle(BASE, (720, 890), (1110, 1080), 0, -1)              # MID
cv2.circle(BASE, SP, 292, 0, -1); cv2.circle(BASE, SP, 200, 255, -1)  # speedo: keep face, drop marker ring
cv2.circle(BASE, SP, 200, 255, -1)

def needle_dir(g, c):
    m = np.zeros_like(g); cv2.circle(m, c, 200, 255, -1); cv2.circle(m, c, 70, 0, -1)
    L = cv2.HoughLinesP(((g > 170) & (m > 0)).astype(np.uint8) * 255, 1, np.pi / 720, 40, minLineLength=70, maxLineGap=8)
    if L is None:
        return None
    x1, y1, x2, y2 = max(L.reshape(-1, 4), key=lambda l: math.hypot(l[2] - l[0], l[3] - l[1]))
    if math.hypot(x1 - c[0], y1 - c[1]) > math.hypot(x2 - c[0], y2 - c[1]):
        x1, y1, x2, y2 = x2, y2, x1, y1
    return math.degrees(math.atan2(y2 - y1, x2 - x1))

def mask_for(ref):
    m = BASE.copy()
    g = cv2.cvtColor(ref, cv2.COLOR_BGR2GRAY)
    for c in (SP, TA):
        a = needle_dir(g, c)
        cv2.circle(m, c, 75, 0, -1)                               # hub
        if a is not None:                                          # wedge +-10 deg around the needle
            pts = [c] + [(int(c[0] + 300 * math.cos(math.radians(a + d))), int(c[1] + 300 * math.sin(math.radians(a + d)))) for d in range(-10, 11, 2)]
            cv2.fillPoly(m, [np.array(pts)], 0)
    return m

def bright(f):
    return cv2.cvtColor(cv2.GaussianBlur(f, (5, 5), 0), cv2.COLOR_BGR2HSV)[..., 2] > 120

def score(ref, on, m):
    new = bright(on) & ~(cv2.dilate(bright(ref).astype(np.uint8), np.ones((11, 11), np.uint8)) > 0) & (m > 0)
    n, lab, st, cen = cv2.connectedComponentsWithStats(new.astype(np.uint8))
    if n <= 1:
        return 0, None, new
    i = 1 + int(np.argmax(st[1:, cv2.CC_STAT_AREA]))
    return int(st[i, cv2.CC_STAT_AREA]), tuple(int(v) for v in cen[i]), new

def save(name, ref, on, new):
    mk = on.copy(); mk[cv2.dilate(new.astype(np.uint8), np.ones((9, 9), np.uint8)) > 0] = (255, 0, 255)
    sm = lambda x: cv2.resize(x[280:1060, 60:1900], (920, 390))
    cv2.imwrite(f"{OUT}/{name}.jpg", np.vstack([sm(ref), sm(on), sm(mk)]))

r = Rig(cam=0)
for c in ("v ign 1", "v light 1", "v fuel 80", "v rpm 2000", "v speed 80", "v cruise 1", "P -", "Q -", "o -"):
    r.cmd(c)
time.sleep(14)
log = csv.writer(open(f"{OUT}/log.csv", "a", newline=""))

def test(label, setup, undo):
    ref = r.frame(); m = mask_for(ref)
    for c in setup: r.cmd(c)
    time.sleep(ON_S)
    on = r.frame()
    for c in undo: r.cmd(c)
    sc, pos, new = score(ref, on, m)
    hit = sc >= MIN_BLOB
    log.writerow([label, sc, pos, int(hit), time.strftime("%H:%M:%S")])
    if hit:
        save(label.replace(" ", "_"), ref, on, new)
        print(f"HIT {label} {sc}px at {pos}", flush=True)
        t0 = time.time()
        while time.time() - t0 < 15 and score(ref, r.frame(), m)[0] >= MIN_BLOB:
            time.sleep(1)
    return sc

try:
    if sys.argv[1] == "noise":
        cv2.imwrite(f"{OUT}/mask.jpg", cv2.resize(cv2.bitwise_and(r.frame(), r.frame(), mask=mask_for(r.frame())), (960, 540)))
        print("noise scores:", [test(f"noise {i}", [], []) for i in range(15)], flush=True)
    elif sys.argv[1] == "b289":
        for idx in (1, 2, 3):
            for v in range(256):
                test(f"289 b{idx} {v:02X}", [f"o 289 {idx} {v:X}"], ["o -"])
            print(f"id 289 byte{idx} done", flush=True)
    else:
        start, end = int(sys.argv[2], 16), int(sys.argv[3], 16)
        for cid in range(start, end + 1):
            if cid in SKIP:
                continue
            best = 0
            for k, (pat, cnt) in enumerate(PATS):
                best = max(best, test(f"{cid:03X} p{k}", [f"Q {cnt}", f"P {cid:X} {pat}"], ["P -"]))
            print(f"id {cid:03X} {best}", flush=True)
    print("DONE", flush=True)
except Exception as e:
    print("ERROR", type(e).__name__, e, flush=True)
    raise
finally:
    try:
        r.cmd("P -"); r.cmd("o -"); r.cmd("v speed 0"); time.sleep(2)
    except Exception:
        pass
    r.close()
