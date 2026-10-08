"""Consistent drive + refuel (sender and 0x2C4 counter tell the same story), camera 1/s.
usage: python consistent.py OUTDIR"""
import sys, os, time, threading, glob, cv2, numpy as np
from fakesimhub import FakeSimHub, fields
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
phase = ["start"]; stop = threading.Event()
def film():
    cam = cv2.VideoCapture(1, cv2.CAP_DSHOW); cam.set(3, 1920); cam.set(4, 1080)
    for _ in range(20): cam.read()
    t0 = time.time(); n = 0
    while not stop.is_set():
        ok, f = cam.read()
        if ok and time.time() - t0 >= n:
            cv2.imwrite(f"{OUT}/{n:04d}_{phase[0]}.jpg", f); n += 1
    cam.release()
sh = FakeSimHub(); time.sleep(8)
th = threading.Thread(target=film, daemon=True); th.start()
def run(name, secs, f0, f1=None, **kw):
    phase[0] = name; print(name, flush=True); t = time.time()
    while time.time() - t < secs:
        x = (time.time() - t) / secs
        fuel = f0 if f1 is None else f0 + (f1 - f0) * x
        sh.send(fields(fuel=round(fuel, 2), **kw)); time.sleep(0.1)
try:
    run("wake", 25, 60, ign=1, speed=0)
    run("drive", 120, 60, 50, ign=1, speed=40, rpm=1500)      # 10 % = 5.2 L used, sender agrees
    run("stop", 10, 50, ign=1, speed=0, hand=1)
    run("refuel", 10, 50, 100, ign=1, speed=0, rpm=0, hand=1) # ETS2 truck 1: ignition stays on
    run("full", 3, 100, ign=1, speed=0, rpm=0, hand=1)
    run("driveoff", 60, 100, ign=1, speed=40, rpm=1500)
finally:
    stop.set(); th.join(5)
    tiles = []
    for p in sorted(glob.glob(f"{OUT}/0*.jpg"))[::4]:
        g = cv2.imread(p)[430:710, 30:350].copy()
        cv2.putText(g, os.path.basename(p)[:-4], (5, 22), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1); tiles.append(g)
    while len(tiles) % 10: tiles.append(np.zeros_like(tiles[0]))
    cv2.imwrite(f"{OUT}/fuel_sheet.jpg", cv2.resize(np.vstack([np.hstack(tiles[k:k+10]) for k in range(0, len(tiles), 10)]), None, fx=0.5, fy=0.5))
    os._exit(0)
