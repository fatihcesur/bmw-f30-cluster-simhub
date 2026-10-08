"""One-off: teach the cluster's long-term consumption average by fake driving.

Distance/fuel steps are multiplied by 2.7 (dmul10 27): ~700 km/h equivalent,
just under the cluster's per-frame plausibility limit.
usage: python teach.py FUEL_PCT CONS_X10 TARGET_RANGE_KM [SPEED] [MAX_MIN]
Drives at SPEED km/h (default 250) with the scanner's calibrated fuel counter
(fk 114, cons = CONS_X10 = l/100km x10) and reads the cluster's own range from
0x330 every 30 s. Stops when the range is within 3 % of the target (or after
MAX_MIN minutes), always bringing the speed back to 0 first (stopping the CAN
traffic while "moving" latches the l/100km needle).
"""
import sys, time, cv2
from scanlib import Rig, CROP

fuel, cons, target = int(sys.argv[1]), int(sys.argv[2]), float(sys.argv[3])
speed = int(sys.argv[4]) if len(sys.argv) > 4 else 250
max_min = float(sys.argv[5]) if len(sys.argv) > 5 else 120

r = Rig(); time.sleep(4)
for c in ("v rpmm 2", "v f3seed 122", "v speed 0", "v rpm 0", f"v fuel {fuel}",
          f"v cons {cons}", "v fk 114", "v ign 1", "v dmul10 27"):
    r.cmd(c)
r.cmd("v mute 8000"); time.sleep(15)          # wake: fuel needle jumps to the real level

def cluster_range():
    r.ser.reset_input_buffer(); r.ser.write(b"s 330\n")
    t0 = time.time(); buf = b""
    while time.time() - t0 < 25:
        buf += r.ser.read(300)
        if buf.count(b"RX 330") >= 1 and buf.endswith(b"\n"): break
    r.ser.write(b"s -1\n"); time.sleep(0.2); r.ser.read(1000)
    for l in reversed(buf.decode(errors="replace").splitlines()):
        p = l.split()
        if len(p) >= 10 and p[0] == "RX" and p[1] == "330":
            return (int(p[9], 16) << 8 | int(p[8], 16)) / 16, int(p[5], 16)
    return None, None

rng, litres = cluster_range()
print(f"start: range {rng} km, cluster fuel {litres} L, target {target}", flush=True)
t0 = time.time()
WAKE_EVERY = 5 * 60                      # the cluster recomputes range only when it wakes
try:
    while time.time() - t0 < max_min * 60:
        r.cmd(f"v speed {speed}"); r.cmd("v rpm 1800")
        time.sleep(WAKE_EVERY)
        r.cmd("v speed 0"); r.cmd("v rpm 0"); time.sleep(3)   # stop before going silent
        r.cmd("v ign 0"); time.sleep(3); r.cmd("v ign 1"); time.sleep(12)   # key cycle: range refresh
        rng, litres = cluster_range()
        mins = (time.time() - t0) / 60
        avg = litres / rng * 100 if rng and litres else None
        print(f"{mins:5.1f} min  range {rng} km  fuel {litres} L  cluster avg {avg and round(avg, 1)} l/100km", flush=True)
        if litres:
            target = litres / (cons / 10) * 100
        if rng and abs(rng - target) / target < 0.05:
            print("TARGET REACHED", flush=True); break
finally:
    r.cmd("v speed 0"); r.cmd("v rpm 0"); time.sleep(5)
    cv2.imwrite("captures/teach_end.jpg", r.frame()[CROP])
    r.close()
