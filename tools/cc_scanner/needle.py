"""Angle of the small l/100km needle (under the tach) in a cropped cluster frame.

usage: python needle.py DIR   -> prints value, angle for every NNN.jpg in DIR
Angle is in degrees, image coordinates: ~165 = pointing at 0 l/100km,
larger = further round towards 10 and 20.
"""
import sys, glob, os, math, cv2, numpy as np

PIVOT = (975, 433)          # needle hub in CROP coordinates (camera index 1 framing)
R_MIN, R_MAX = 22, 46       # needle body, inside the "l/100km" label
FUEL_PIVOT = (150, 232)     # fuel gauge hub in CROP coordinates
FUEL_R = (18, 60)

def angle(f, pivot=PIVOT, rr=(R_MIN, R_MAX), below=-25):
    # max channel, not grey: in the red night theme the needle is pure red
    g = f.max(axis=2)
    thr = max(120, int(np.percentile(g, 99.5)) - 40)
    ys, xs = np.nonzero(g > thr)
    dx, dy = xs - pivot[0], ys - pivot[1]
    r = np.hypot(dx, dy)
    keep = (r > rr[0]) & (r < rr[1]) & (dy > below)
    if keep.sum() < 15:
        return None
    a = np.degrees(np.arctan2(dy[keep], dx[keep])) % 360
    return round(float(np.median(a)), 1)

def fuel_angle(f):
    return angle(f, FUEL_PIVOT, FUEL_R, below=-999)

if __name__ == "__main__":
    for p in sorted(glob.glob(os.path.join(sys.argv[1], "[0-9]*.jpg")) + glob.glob(os.path.join(sys.argv[1], "baseline.jpg"))):
        print(os.path.basename(p)[:-4], angle(cv2.imread(p)))
