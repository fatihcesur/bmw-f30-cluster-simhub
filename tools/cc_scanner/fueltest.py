"""Long-running check of what drives the l/100km needle: hold each setting for
HOLD seconds at a fake 80 km/h / 2000 rpm and log the needle angle every 5 s.
Settings: old per-Loop count, then the fk fuel counter with cons=100 (10.0 l/100km)
at a few factors. Photos + log in captures/fueltest/."""
import time, csv, os, cv2
from scanlib import Rig, CROP
from needle import angle

HOLD = 60
os.makedirs("captures/fueltest", exist_ok=True)
r = Rig(); time.sleep(4)
r.cmd("v speed 80"); r.cmd("v rpm 2000"); r.cmd("v cons 100")
log = csv.writer(open("captures/fueltest/log.csv", "w", newline=""))
try:
    for name, cmds in [("fk1", ["v fk 1"]), ("count", ["v fk 0"]), ("fk1b", ["v fk 1"]),
                       ("fk300", ["v fk 300"]), ("fk3000", ["v fk 3000"])]:
        for c in cmds: r.cmd(c)
        t0 = time.time()
        while time.time() - t0 < HOLD:
            time.sleep(5)
            f = r.frame()[CROP]; a = angle(f); t = int(time.time() - t0)
            log.writerow([name, t, a]); print(name, t, a, flush=True)
        cv2.imwrite(f"captures/fueltest/{name}.jpg", f)
finally:
    r.cmd("v fk 0"); r.cmd("v speed 0"); r.cmd("v rpm 0"); r.close()
