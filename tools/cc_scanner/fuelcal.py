"""Fuel gauge calibration: for each raw 0x349 value, wake the cluster (8 s CAN
silence -> needle jumps to the level) and photograph/measure the needle."""
import time, cv2, numpy as np
from scanlib import Rig, CROP
from needle import fuel_angle
r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v speed 0", "v rpm 0", "v ign 1"): r.cmd(c)
shots = []
for raw in list(range(0, 41, 2)) + [45, 50, 60]:
    r.cmd(f"v fraw {raw * 256}"); r.cmd("v mute 8000"); time.sleep(17)
    f = r.frame()[CROP]; a = fuel_angle(f)
    cv2.imwrite(f"captures/fuelcal/raw{raw:02d}.jpg", f)
    im = cv2.resize(f[100:360, 20:300], None, fx=0.8, fy=0.8); cv2.putText(im, f"raw {raw} {a}", (5, 22), 0, 0.55, (0, 255, 0), 2); shots.append(im)
    print("raw", raw, "angle", a, flush=True)
r.cmd("v fraw -1"); r.close()
shots += [np.zeros_like(shots[0])] * (-len(shots) % 6)
cv2.imwrite("captures/fuelcal/sheet.jpg", np.vstack([np.hstack(shots[i:i+6]) for i in range(0, len(shots), 6)]))
