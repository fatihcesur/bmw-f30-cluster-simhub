"""Replay what ETS2 did in-game on 2026-10-04 18:44-18:49 (game2/game.csv) against the
real firmware: truck switch through a loading screen (SimHub sends fuel 0 for ~15 s),
half tank, then a refuel with the ignition OFF (5 s fill, slow start) and drive-off.

usage: python switchtest.py OUTDIR      -> OUTDIR/NNNN_<phase>.jpg (1/s) + fuel_sheet.jpg
"""
import sys, os, time, threading, cv2, numpy as np
from fakesimhub import FakeSimHub, fields

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
phase = ["start"]; stop = threading.Event()

def film():
    cam = cv2.VideoCapture(1, cv2.CAP_DSHOW)
    cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920); cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
    for _ in range(20): cam.read()
    t0 = time.time(); n = 0
    while not stop.is_set():
        ok, f = cam.read()
        if ok and time.time() - t0 >= n:
            cv2.imwrite(f"{OUT}/{n:04d}_{phase[0]}.jpg", f); n += 1
    cam.release()

sh = FakeSimHub()
time.sleep(8)                       # let the cluster sleep so the test starts with a wake
th = threading.Thread(target=film, daemon=True); th.start()

def run(name, secs, f0, f1=None, **kw):
    phase[0] = name; print(name, flush=True); t = time.time()
    while time.time() - t < secs:
        x = (time.time() - t) / secs
        fuel = f0 if f1 is None else round(f0 + (f1 - f0) * x)
        sh.send(fields(fuel=fuel, **kw)); time.sleep(0.1)

try:
    run("old", 40, 99, ign=1, speed=0)                          # old truck, nearly full
    run("loading", 15, 0, ign=0, speed=0, rpm=0)                # SimHub: no data -> 0
    run("newoff", 10, 48, ign=0, speed=0, rpm=0)                # new truck, half tank
    run("newon", 25, 48, ign=1, speed=0)
    run("drive", 20, 48, ign=1, speed=40, rpm=1500)
    run("stop", 5, 48, ign=1, speed=0, hand=1)
    run("ignoff", 4, 48, ign=0, speed=0, rpm=0, hand=1)
    run("refuel", 5, 48, 100, ign=0, speed=0, rpm=0, hand=1)    # in-game: ~5 s, ignition off
    run("full", 6, 100, ign=0, speed=0, rpm=0, hand=1)
    run("ignon", 6, 100, ign=1, speed=0, hand=1)
    run("driveoff", 40, 100, ign=1, speed=40, rpm=1500)
    run("park", 5, 100, ign=1, speed=0)
finally:
    stop.set(); th.join(5)
    tiles = []
    import glob
    for p in sorted(glob.glob(f"{OUT}/0*.jpg"))[::3]:
        g = cv2.imread(p)[430:710, 30:350].copy()
        cv2.putText(g, os.path.basename(p)[:-4], (5, 22), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
        tiles.append(g)
    while len(tiles) % 10: tiles.append(np.zeros_like(tiles[0]))
    cv2.imwrite(f"{OUT}/fuel_sheet.jpg", cv2.resize(np.vstack([np.hstack(tiles[k:k + 10]) for k in range(0, len(tiles), 10)]), None, fx=0.5, fy=0.5))
    os._exit(0)
