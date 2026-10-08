"""Consumption needle vs 0x2C4 counter and 0xF3 spare bytes, with a real (valid
0xF3) engine speed. Each state is held and the needle angle logged every 5 s."""
import time, csv, os, cv2
from scanlib import Rig, CROP
from needle import angle

os.makedirs("captures/fueltest2", exist_ok=True)
r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v speed 80", "v rpm 2000", "v cons 100"): r.cmd(c)
states = [("count", ["v fk 0"], 40), ("fk1", ["v fk 1"], 40), ("fk3000", ["v fk 3000"], 40),
          ("fk30000", ["v fk 30000"], 40), ("fk0again", ["v fk 0"], 20)]
states += [(f"f3b5_{v:02X}", [f"o f3 5 {v:x}"], 20) for v in (0x00, 0x40, 0x80, 0xC4, 0xFF)]
states += [(f"f3b3_{v:02X}", ["o -", f"o f3 3 {v:x}"], 20) for v in (0x00, 0x40, 0x80, 0xFF)]
states += [(f"f3b4_{v:02X}", ["o -", f"o f3 4 {v:x}"], 20) for v in (0x00, 0x80, 0xF6, 0xFF)]
log = csv.writer(open("captures/fueltest2/log.csv", "w", newline=""))
try:
    for name, cmds, hold in states:
        for c in cmds: r.cmd(c)
        t0 = time.time(); vals = []
        while time.time() - t0 < hold:
            time.sleep(5); f = r.frame()[CROP]; a = angle(f); vals.append(a)
            log.writerow([name, int(time.time() - t0), a])
        cv2.imwrite(f"captures/fueltest2/{name}.jpg", f)
        print(name, vals, flush=True)
finally:
    for c in ("o -", "v fk 0", "v speed 0", "v rpm 0"): r.cmd(c)
    r.close()
