"""Sweep one byte of a frame the firmware already sends and photograph each value.

usage: python sweep.py MODE [SPEED] [STEP]
  SPEED  fake vehicle speed in km/h during the sweep (default 0; cruise needs motion)
  STEP   value step (default 1, cruise default 4)
  lights0   0x21A byte0 = 0..255 (byte1 0x00)      low-beam / fog / parking icons
  lights1   0x21A byte1 = 0..255 (byte0 0x05)      day/night theme candidates
  back1     0x202 byte1 = 0..255                   ambient-light / theme candidates
  cruiseN   0x289 byte N (1..6) = 0..255 step 4, cruise ON   set-speed candidates
  c2c4N     0x2C4 byte N (0..6) = 0..255, 5 s per value      consumption candidates

Every value gets a full-frame photo in captures/sweep_MODE/, values whose frame
differs from the baseline get a side-by-side in captures/sweep_MODE/hits/, and
captures/sweep_MODE/log.csv lists value, changed pixels, hit.
"""
import sys, os, time, csv, cv2, numpy as np
from scanlib import Rig
from scanlib import diff_score, CROP, PIX_THRESHOLD

MODE = sys.argv[1]
SPEED = int(sys.argv[2]) if len(sys.argv) > 2 else 0
STEP = int(sys.argv[3]) if len(sys.argv) > 3 else (4 if MODE.startswith("cruise") else 1)
WAIT = 5.0 if MODE.startswith("c2c4") else 1.5          # the consumption needle is heavily damped

def plan():
    if MODE == "lights0":
        return ["L 05 00"], [(v, [f"L {v:02x} 00"]) for v in range(0, 256, STEP)], ["L -"]
    if MODE == "lights1":
        return ["L 05 00"], [(v, [f"L 05 {v:02x}"]) for v in range(0, 256, STEP)], ["L -"]
    if MODE == "back1":
        return ["o -"], [(v, [f"o 202 1 {v:x}"]) for v in range(0, 256, STEP)], ["o -"]
    if MODE.startswith("c2c4"):
        idx = int(MODE[4:])
        return ["o -"], [(v, [f"o 2c4 {idx} {v:x}"]) for v in range(0, 256, STEP)], ["o -"]
    if MODE.startswith("cruise"):
        idx = int(MODE[6:])
        return ["o -", "v cruise 1"], [(v, [f"o 289 {idx} {v:x}"]) for v in range(0, 256, STEP)], ["o -", "v cruise 0"]
    sys.exit("unknown mode")

setup, steps, teardown = plan()
out = f"captures/sweep_{MODE}" + (f"_{SPEED}kmh" if SPEED else "")
os.makedirs(out + "/hits", exist_ok=True)
r = Rig()
time.sleep(5)
r.cmd("v btn 1"); time.sleep(0.3); r.cmd("v btn 0")   # dismiss a lingering popup
for c in setup + ([f"v speed {SPEED}", "v rpm 2000"] if SPEED else []): r.cmd(c)
time.sleep(6 if SPEED else 3)                            # let the needle get there
base = r.frame()[CROP]
cv2.imwrite(f"{out}/baseline.jpg", base)
log = open(f"{out}/log.csv", "w", newline=""); w = csv.writer(log)
try:
    for v, cmds in steps:
        for c in cmds: r.cmd(c)
        time.sleep(WAIT)
        f = r.frame()[CROP]
        score, mask = diff_score(base, f)
        hit = score >= PIX_THRESHOLD
        cv2.imwrite(f"{out}/{v:03d}.jpg", f, [cv2.IMWRITE_JPEG_QUALITY, 80])
        if hit:
            m = f.copy(); m[mask] = (0, 255, 255)
            cv2.imwrite(f"{out}/hits/{v:03d}.jpg", np.hstack([base, f, m]))
        w.writerow([v, score, int(hit)]); log.flush()
        print(MODE, v, score, "HIT" if hit else "", flush=True)
except Exception as e:
    print("ERROR", type(e).__name__, e, flush=True)
    raise
finally:
    try:
        for c in teardown + (["v speed 0", "v rpm 0"] if SPEED else []): r.cmd(c)
    except Exception: pass
    r.close(); log.close()
