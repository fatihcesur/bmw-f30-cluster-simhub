"""Hunt for the cruise-control set-speed marker: a green/orange LED on an arm that runs
around the speedometer scale (white dial LEDs since 2026-10-08, so orange stands out).

The cluster is held at a fake 80 km/h with the cruise icon on. Phase A sweeps every byte
of our own 0x289 cruise frame; phase B sends periodic frames on every other CAN id. After
each step the webcam frame is checked for NEW saturated green/orange pixels in a ring
around the speedometer scale, compared with a reference frame.

usage: python cruisehunt.py calib            reference frame + ROI overlay only
       python cruisehunt.py run [START END]  phase A, then phase B over START..END (hex)
Output: captures/cruise/ (hits as ref | on | marked, log.csv). Lines: "HIT ...", "id ...",
"PHASE ...", "DONE", "ERROR ...". Requires the cc_scanner firmware.
"""
import sys, os, time, csv, cv2, numpy as np
from scanlib import Rig

CAM = 0
CX, CY, R_IN, R_OUT = 564, 690, 200, 292      # speedometer scale ring (Brio, framing 2026-10-09)
MIN_BLOB = 25                                  # new coloured pixels in one blob to call it a hit
ON_S = 1.6
OUT = "captures/cruise"
os.makedirs(OUT, exist_ok=True)

SKIP = {0x0D7, 0x0F3, 0x12F, 0x193, 0x19B, 0x1A1, 0x1EE, 0x1F6, 0x202, 0x21A, 0x289,
        0x291, 0x297, 0x2A7, 0x2BB, 0x2C3, 0x2C4, 0x349, 0x34F, 0x368, 0x36E, 0x39E,
        0x3A7, 0x3F9, 0x3FD, 0x5C0,
        0x1B3, 0x205, 0x2C5, 0x2CA, 0x2F7, 0x328, 0x330, 0x336, 0x338, 0x35C, 0x362,
        0x367, 0x393, 0x560, 0x5E0, 0x660}
PATS = [("50 50 50 50 50 50 50 50", "-"), ("00 F0 50 50 50 50 50 50", "1"), ("FF FF FF FF FF FF FF FF", "-")]

ROI = np.zeros((1080, 1920), np.uint8)
cv2.circle(ROI, (CX, CY), R_OUT, 255, -1)
cv2.circle(ROI, (CX, CY), R_IN, 0, -1)

def colour(f):
    h = cv2.cvtColor(cv2.GaussianBlur(f, (5, 5), 0), cv2.COLOR_BGR2HSV)
    H, S, V = h[..., 0], h[..., 1], h[..., 2]
    green = (H >= 40) & (H <= 90) & (S >= 90) & (V >= 90)
    orange = (H >= 4) & (H <= 22) & (S >= 140) & (V >= 120)
    return (green | orange) & (ROI > 0), green, orange

def score(ref, on):
    m_ref = colour(ref)[0]
    m_on, g, o = colour(on)
    new = m_on & ~(cv2.dilate(m_ref.astype(np.uint8), np.ones((15, 15), np.uint8)) > 0)
    n, lab, st, _ = cv2.connectedComponentsWithStats(new.astype(np.uint8))
    if n <= 1:
        return 0, "", new
    i = 1 + int(np.argmax(st[1:, cv2.CC_STAT_AREA]))
    blob = lab == i
    kind = "green" if (blob & g).sum() >= (blob & o).sum() else "orange"
    return int(st[i, cv2.CC_STAT_AREA]), kind, new

def save(name, ref, on, new):
    marked = on.copy(); marked[cv2.dilate(new.astype(np.uint8), np.ones((9, 9), np.uint8)) > 0] = (255, 0, 255)
    crop = (slice(CY - R_OUT - 20, CY + R_OUT + 20), slice(CX - R_OUT - 20, CX + R_OUT + 20))
    cv2.imwrite(f"{OUT}/{name}.jpg", np.hstack([ref[crop], on[crop], marked[crop]]))

r = Rig(cam=CAM)
for c in ("v ign 1", "v light 1", "v fuel 80", "v rpm 2000", "v speed 80", "v cruise 1", "P -", "Q -", "o -"):
    r.cmd(c)
time.sleep(12)
ref = r.frame()

if sys.argv[1] == "calib":
    cv2.imwrite(f"{OUT}/calib_ref.jpg", ref)
    ov = ref.copy(); ov[ROI > 0] = (ov[ROI > 0] * 0.6 + np.array([0, 80, 0])).astype(np.uint8)
    m = colour(ref)[0]; ov[m] = (255, 0, 255)
    cv2.imwrite(f"{OUT}/calib_roi.jpg", ov)
    print("coloured pixels already in ROI:", int(m.sum()), flush=True)
    r.cmd("v speed 0"); time.sleep(1); r.close(); sys.exit()

log = csv.writer(open(f"{OUT}/log.csv", "a", newline=""))

def step(label, ref):
    on = r.frame()
    sc, kind, new = score(ref, on)
    hit = sc >= MIN_BLOB
    log.writerow([label, sc, kind, int(hit), time.strftime("%H:%M:%S")])
    if hit:
        save(label.replace(" ", "_"), ref, on, new)
        print(f"HIT {label} {kind} {sc}px", flush=True)
    return hit, sc

try:
    print("PHASE A 0x289 byte sweep", flush=True)
    for idx in range(1, 7):
        best = 0
        for val in range(0, 256, 8):
            r.cmd(f"o 289 {idx} {val:X}")
            time.sleep(ON_S)
            hit, sc = step(f"289 b{idx} {val:02X}", ref)
            best = max(best, sc)
        r.cmd("o -"); time.sleep(3)
        print(f"id 289 byte{idx} {best}", flush=True)
    start = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x001
    end = int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x5FF
    print(f"PHASE B ids {start:03X}-{end:03X}", flush=True)
    for cid in range(start, end + 1):
        if cid in SKIP:
            continue
        best = 0
        for k, (pat, cnt) in enumerate(PATS):
            ref = r.frame()
            r.cmd(f"Q {cnt}"); r.cmd(f"P {cid:X} {pat}")
            time.sleep(ON_S)
            hit, sc = step(f"{cid:03X} p{k}", ref)
            r.cmd("P -")
            best = max(best, sc)
            if hit:
                t0 = time.time()
                while time.time() - t0 < 20 and score(ref, r.frame())[0] >= MIN_BLOB:
                    time.sleep(1)
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
