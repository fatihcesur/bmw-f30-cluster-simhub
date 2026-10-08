"""Brute-force the 0x2C4 CRC seed for a few alive-counter layouts. Frame bytes 2-7
are 0xFF; if the cluster starts accepting the frame and any of those bytes is a
consumption signal, the l/100km needle should leave 0. One Arduino session only
(restarting while "moving" latches the needle at 20)."""
import time, cv2, os, sys
from scanlib import Rig, CROP
from needle import angle

hdrs = [int(h, 16) for h in sys.argv[1:]] or [0xF0, 0x60, 0x10]
os.makedirs("captures/seedscan", exist_ok=True)
r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v speed 80", "v rpm 2000"): r.cmd(c)
time.sleep(8)
try:
    for hdr in hdrs:
        r.cmd(f"v c4hdr {hdr}")
        for seed in range(256):
            r.cmd(f"v c4seed {seed}"); time.sleep(3.5)
            f = r.frame()[CROP]; a = angle(f)
            hit = a is not None and a < 160
            if hit: cv2.imwrite(f"captures/seedscan/hdr{hdr:02X}_seed{seed:02X}.jpg", f)
            print(f"hdr {hdr:02X} seed {seed:02X} {a}{' HIT' if hit else ''}", flush=True)
finally:
    for c in ("v c4hdr 0", "v speed 0", "v rpm 0"): r.cmd(c)
    time.sleep(2); r.close()
