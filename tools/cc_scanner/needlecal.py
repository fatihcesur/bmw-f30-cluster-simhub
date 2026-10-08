"""Measure where the speedometer and tachometer needles really point (Brio, head-on).

Input: captures/needlecal/s<kmh>_r<rpm>.jpg (one frame per step, needles moving together).
1. Each frame: the needle is the longest bright straight line near the dial centre.
2. Dial centre = least-squares intersection of all needle lines of that dial.
3. Tick marks = bright spots on a ring in the per-pixel MINIMUM of all frames (needle gone).
4. Needle angle -> dial value by interpolating between neighbouring ticks.
Prints sent vs shown and writes a check image per dial.
usage: python needlecal.py
"""
import glob, math, re, cv2, numpy as np

FILES = sorted(glob.glob("captures/needlecal/s*_r*.jpg"))
DIALS = {
    # name: (centre guess, needle r range, tick ring r range, first/last tick value, tick step, sent index in name)
    "speed": dict(c=(555, 715), rn=(70, 200), rt=(208, 244), v0=0, v1=260, step=10, key=0),
    "rpm": dict(c=(1226, 731), rn=(70, 200), rt=(212, 250), v0=0, v1=6000, step=500, key=1),
}

def sent(path, key):
    s, r = re.findall(r"s(\d+)_r(\d+)", path)[0]
    return int((s, r)[key])

def needle_line(img, c, rn):
    g = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    m = np.zeros_like(g); cv2.circle(m, c, rn[1], 255, -1); cv2.circle(m, c, rn[0], 0, -1)
    b = ((g > 170) & (m > 0)).astype(np.uint8) * 255
    lines = cv2.HoughLinesP(b, 1, np.pi / 720, 40, minLineLength=70, maxLineGap=8)
    if lines is None:
        return None
    best = None
    for x1, y1, x2, y2 in lines.reshape(-1, 4):
        d = abs((x2 - x1) * (c[1] - y1) - (y2 - y1) * (c[0] - x1)) / math.hypot(x2 - x1, y2 - y1)
        L = math.hypot(x2 - x1, y2 - y1)
        if d < 40 and (best is None or L > best[0]):
            best = (L, (x1, y1, x2, y2))
    return best[1] if best else None

def intersect(lines):
    A, b = [], []
    for x1, y1, x2, y2 in lines:
        n = np.array([y2 - y1, -(x2 - x1)], float); n /= np.linalg.norm(n)
        A.append(n); b.append(n @ [x1, y1])
    return np.linalg.lstsq(np.array(A), np.array(b), rcond=None)[0]

def angle(p, c):
    return math.degrees(math.atan2(p[1] - c[1], p[0] - c[0])) % 360

imgs = [cv2.imread(f) for f in FILES]
mn = np.min(np.stack([cv2.cvtColor(i, cv2.COLOR_BGR2GRAY) for i in imgs]), axis=0)

for name, d in DIALS.items():
    lines = [needle_line(i, d["c"], d["rn"]) for i in imgs]
    ok = [l for l in lines if l is not None]
    c = intersect(ok)
    # tick angles: bright ring pixels in the needle-free minimum image, clustered by angle
    prof = np.zeros(3600)
    for a10 in range(3600):
        a = math.radians(a10 / 10)
        vals = [mn[int(round(c[1] + r * math.sin(a))), int(round(c[0] + r * math.cos(a)))] for r in range(d["rt"][0], d["rt"][1], 2)]
        prof[a10] = max(vals) if not (40 <= a10 / 10 <= 145) else 0   # bottom: no scale (l/100km gauge)
    on = prof > 110
    ticks = []
    a10 = 0
    while a10 < 3600:
        if on[a10]:
            s = a10
            while a10 < 3600 and on[a10]:
                a10 += 1
            ticks.append((s + a10 - 1) / 20.0)
        a10 += 1
    nt = round((d["v1"] - d["v0"]) / d["step"]) + 1
    print(f"\n{name}: centre {c.round(1)}, {len(ok)}/{len(imgs)} needles, {len(ticks)} tick blobs (expect {nt})")
    print("  tick angles:", [round(t, 1) for t in ticks])
    # order ticks along the dial: scale runs clockwise (increasing image angle) from the
    # lower left; unwrap so the gap (bottom of the dial) is the cut
    if name == "rpm":                       # red-zone hatching confuses the ring: use 500..5000
        ticks = [a for a in ticks if 180 <= a <= 356]
        d = dict(d, v0=500, v1=5000)
        nt = 10
    t = sorted(ticks)
    gaps = [((t[(i + 1) % len(t)] - t[i]) % 360, i) for i in range(len(t))]
    cut = max(gaps)[1]
    seq = t[cut + 1:] + [x + 360 for x in t[:cut + 1]]
    if len(seq) != nt:
        print("  !! tick count mismatch, check calib image")
    vals = [d["v0"] + i * d["step"] for i in range(len(seq))]
    out = imgs[len(imgs) // 2].copy()
    for a, v in zip(seq, vals):
        p = (int(c[0] + 245 * math.cos(math.radians(a))), int(c[1] + 245 * math.sin(math.radians(a))))
        cv2.circle(out, p, 4, (0, 0, 255), -1)
    for f, l in zip(FILES, lines):
        sv = sent(f, d["key"])
        if l is None:
            print(f"  sent {sv:5d}: needle not found"); continue
        x1, y1, x2, y2 = l
        tip = (x1, y1) if math.hypot(x1 - c[0], y1 - c[1]) > math.hypot(x2 - c[0], y2 - c[1]) else (x2, y2)
        a = angle(tip, c)
        if a < seq[0] - 20:
            a += 360
        shown = float(np.interp(a, seq, vals))
        print(f"  sent {sv:5d}: angle {a:6.1f} -> shown {shown:7.1f}  ({shown - sv:+.1f})")
    cv2.imwrite(f"captures/needlecal/check_{name}.jpg", out)
