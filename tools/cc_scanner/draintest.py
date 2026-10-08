"""Does a slowly falling 0x349 fuel level move the l/100km needle? Fake 80 km/h,
fuel sent as a fine 16-bit value, drained at several rates (raw units per 10 s)."""
import time, csv, sys, cv2
from scanlib import Rig, CROP
from needle import angle

rates = [int(x) for x in sys.argv[1:]] or [0, 5, 20, 80, 300, 1000]
HOLD = 45
r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v fraw 5120", "v fdrain 0", "v speed 80", "v rpm 2000"): r.cmd(c)
time.sleep(15)
log = csv.writer(open("captures/drain/log.csv", "a", newline=""))
try:
    for rate in rates:
        r.cmd(f"v fdrain {rate}"); t0 = time.time(); vals = []
        while time.time() - t0 < HOLD:
            time.sleep(5); f = r.frame()[CROP]; a = angle(f); vals.append(a)
            log.writerow([rate, int(time.time() - t0), a])
        cv2.imwrite(f"captures/drain/rate{rate}.jpg", f)
        print("drain", rate, vals, flush=True)
finally:
    for c in ("v fdrain 0", "v speed 0", "v rpm 0", "v fraw -1"): r.cmd(c)
    r.close()
