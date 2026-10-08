"""Scan 0x5c0 check-control codes one at a time and photograph the cluster.

usage: python scan.py START END
Every code: code ON, wait -> frame -> code OFF -> wait until the cluster is idle
again (matches the idle reference, or stops changing), max 30 s.
All ON frames are saved (cropped) to captures/all/, codes whose ON frame differs
from the idle reference go to captures/hits/ (side-by-side) and captures/mid/
(zoomed MID), and one line per code is appended to captures/scan_log.csv
(code, changed pixels, hit, settle seconds; negative = settled on a new idle look).
"""
import sys, os, time, csv, cv2, numpy as np
from scanlib import Rig, diff_score, CROP, PIX_THRESHOLD

START, END = int(sys.argv[1]), int(sys.argv[2])
ON_WAIT = 2.5
os.makedirs("captures/all", exist_ok=True); os.makedirs("captures/hits", exist_ok=True); os.makedirs("captures/mid", exist_ok=True)

r = Rig()
time.sleep(5)
ref = r.frame()[CROP]                            # idle cluster, no test code active
cv2.imwrite("captures/reference.jpg", ref)
SETTLE_MAX = 30.0

def wait_settled():
    """After OFF, wait until the cluster is idle again: either it matches the idle
    reference, or it has stopped changing for a few seconds (slow brightness drift
    of the backlight/camera makes an old reference stop matching over time)."""
    global ref
    t0 = time.time(); prev = None; stable = 0
    while time.time() - t0 < SETTLE_MAX:
        cur = r.frame()[CROP]
        if diff_score(ref, cur)[0] < PIX_THRESHOLD:
            ref = cur
            return round(time.time() - t0, 1), cur
        stable = stable + 1 if prev is not None and diff_score(prev, cur)[0] < PIX_THRESHOLD // 2 else 0
        if stable >= 6 and time.time() - t0 > 4:     # ~3 s without change
            ref = cur
            return -round(time.time() - t0, 1), cur  # negative = settled on a new idle look
        prev = cur
        time.sleep(0.5)
    return -99, cur

log = open("captures/scan_log.csv", "a", newline=""); w = csv.writer(log)
try:
    for code in range(START, END + 1):
        r.cmd(f"c {code}"); time.sleep(ON_WAIT)
        on = r.frame()[CROP]
        r.cmd("x")
        score, mask = diff_score(ref, on)
        settle, cur = wait_settled()
        cv2.imwrite(f"captures/all/{code:05d}.jpg", on, [cv2.IMWRITE_JPEG_QUALITY, 80])
        hit = score >= PIX_THRESHOLD
        if hit:
            marked = on.copy(); marked[mask] = (0, 255, 255)
            cv2.imwrite(f"captures/hits/{code:05d}.jpg", np.hstack([ref, on, marked]))
            zoom = cv2.resize(on[360:600, 180:820], None, fx=2, fy=2, interpolation=cv2.INTER_CUBIC)
            cv2.putText(zoom, f"code {code}", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 2)
            cv2.imwrite(f"captures/mid/{code:05d}.jpg", zoom)
        if not hit:
            ref = on                             # nothing shown: this is the freshest idle look
        if settle == -99:                        # never settled, keep the photo
            cv2.imwrite(f"captures/hits/{code:05d}_stuck.jpg", cur)
            pass
        w.writerow([code, score, int(hit), settle, time.strftime("%H:%M:%S")]); log.flush()
        print(code, score, "HIT" if hit else "", "settle", settle, flush=True)
except Exception as e:
    print("ERROR", type(e).__name__, e, flush=True)
    raise
finally:
    try: r.cmd("x")
    except Exception: pass
    r.close(); log.close()
