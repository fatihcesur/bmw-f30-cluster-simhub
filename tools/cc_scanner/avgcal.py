"""Calibrate the 0x2C4 fuel counter: reset the MID average (long MENU press on the
l/100km page, which must be showing), drive at a fake speed with a given fk/cons,
photograph the MID average every 20 s.  usage: python avgcal.py FK CONS SPEED SECS"""
import sys, time, cv2, numpy as np
from scanlib import Rig, CROP
fk, cons, speed, secs = (int(x) for x in sys.argv[1:5])
r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v speed 0", "v rpm 0", "v fuel 100", f"v cons {cons}", f"v fk {fk}"): r.cmd(c)
def press(hold=0.3, after=2.5):
    r.cmd("v btn 1"); time.sleep(hold); r.cmd("v btn 0"); time.sleep(after)
shots = []
def mid(label):
    f = r.frame()[CROP]; im = cv2.resize(f[380:600, 480:900], None, fx=0.8, fy=0.8)
    cv2.putText(im, label, (5, 20), 0, 0.5, (0, 255, 0), 2); shots.append(im)
try:
    r.cmd(f"v speed {speed}"); r.cmd("v rpm 2000"); time.sleep(5)
    press(hold=3.5, after=1); t0 = time.time()
    while time.time() - t0 < secs:
        time.sleep(20); mid(f"fk{fk} cons{cons} {speed}kmh +{int(time.time()-t0)}s")
finally:
    r.cmd("v speed 0"); r.cmd("v rpm 0"); time.sleep(2)
r.close()
shots += [np.zeros_like(shots[0])] * (-len(shots) % 5)
out = f"captures/drain/avgcal_fk{fk}_c{cons}_s{speed}.jpg"
cv2.imwrite(out, np.vstack([np.hstack(shots[i:i+5]) for i in range(0, len(shots), 5)])); print(out)
