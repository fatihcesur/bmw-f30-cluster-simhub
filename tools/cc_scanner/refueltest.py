"""Replay an ETS2 refuel stop against the REAL firmware (fake SimHub) and film the fuel needle.

usage: python refueltest.py OUTDIR [ramp_seconds] [ign_off 1|0] [ign_on_seconds] [full_seconds]
Phases: settle at 15 % parked -> drive in -> stop -> ignition off -> fuel ramps
15 -> 100 % (ETS2 fills gradually) -> ignition on -> drive off at once.
One full frame per second goes to OUTDIR/NNNN_<phase>.jpg, plus phases.csv.
"""
import sys, os, time, threading, csv, cv2
from fakesimhub import FakeSimHub, fields

OUT = sys.argv[1]; RAMP = float(sys.argv[2]) if len(sys.argv) > 2 else 15
IGN_OFF = int(sys.argv[3]) if len(sys.argv) > 3 else 1   # 0: ignition stays on during the stop
IGN_ON_S = float(sys.argv[4]) if len(sys.argv) > 4 else 5  # seconds parked after ignition-on
FULL_S = float(sys.argv[5]) if len(sys.argv) > 5 else 12   # seconds parked after the fill
os.makedirs(OUT, exist_ok=True)
phase = ["start"]; stop = threading.Event()

def film():
    cam = cv2.VideoCapture(1, cv2.CAP_DSHOW)
    cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920); cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
    cam.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0.25); cam.set(cv2.CAP_PROP_EXPOSURE, -5)
    for _ in range(20): cam.read()
    t0 = time.time(); n = 0
    while not stop.is_set():
        ok, f = cam.read()
        if ok and time.time() - t0 >= n:
            cv2.imwrite(f"{OUT}/{n:04d}_{phase[0]}.jpg", f); n += 1
    cam.release()

sh = FakeSimHub()
time.sleep(8)        # firmware is silent until the first packet: let the cluster sleep, so it wakes at 15 %
th = threading.Thread(target=film, daemon=True); th.start()
log = csv.writer(open(f"{OUT}/phases.csv", "w", newline=""))
T0 = time.time()

def run(name, secs, fuel_from, fuel_to=None, **kw):
    phase[0] = name; log.writerow([round(time.time() - T0, 1), name]); print(name, flush=True)
    t = time.time()
    while time.time() - t < secs:
        x = (time.time() - t) / secs
        fuel = fuel_from if fuel_to is None else round(fuel_from + (fuel_to - fuel_from) * x)
        sh.send(fields(fuel=fuel, **kw)); time.sleep(0.1)

try:
    run("settle", 60, 15, ign=1, speed=0)
    run("drive", 10, 15, ign=1, speed=40, rpm=1500)
    run("stop", 5, 15, ign=1, speed=0, hand=1)
    run("ignoff", 5, 15, ign=1 - IGN_OFF, speed=0, rpm=0, hand=1)
    run("refuel", RAMP, 15, 100, ign=1 - IGN_OFF, speed=0, rpm=0, hand=1)
    run("full", FULL_S, 100, ign=1 - IGN_OFF, speed=0, rpm=0, hand=1)
    run("ignon", IGN_ON_S, 100, ign=1, speed=0, hand=1)
    run("driveoff", 60, 100, ign=1, speed=50, rpm=1600)
    run("park", 10, 100, ign=1, speed=0)
finally:
    stop.set(); th.join(5)
