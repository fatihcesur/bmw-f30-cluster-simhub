"""Log what SimHub sees from ETS2 (5 Hz, via SimHub's web API) and film the fuel gauge
automatically when a refuel starts (fuel rising while standing still).

usage: python gamelog.py OUTDIR
OUTDIR/game.csv: time, fuel %, fuel L, ignition, engine, speed, gear, paused
OUTDIR/refuel_<HHMMSS>/: one cluster frame per second for 120 s after a refuel starts,
plus sheet.jpg (fuel gauge crops). Prints REFUEL / SAVED lines (for a Monitor).
"""
import sys, os, time, json, csv, threading, urllib.request
import cv2, numpy as np

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
URL = "http://localhost:8888/api/getgamedata"

def sample():
    d = json.load(urllib.request.urlopen(URL, timeout=2))
    n = d.get("NewData") or {}
    return (n.get("FuelPercent"), n.get("Fuel"), n.get("EngineIgnitionOn"), n.get("EngineStarted"),
            n.get("SpeedKmh"), n.get("Gear"), d.get("GamePaused"))

def film(folder, secs=120):
    os.makedirs(folder, exist_ok=True)
    cam = cv2.VideoCapture(1, cv2.CAP_DSHOW)
    cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920); cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
    for _ in range(15): cam.read()
    tiles = []; t0 = time.time(); n = 0
    while time.time() - t0 < secs:
        ok, f = cam.read()
        if ok and time.time() - t0 >= n:
            cv2.imwrite(f"{folder}/{n:03d}.jpg", f, [cv2.IMWRITE_JPEG_QUALITY, 80])
            if n % 3 == 0:
                g = f[430:710, 30:350].copy()         # fuel gauge (camera upright)
                cv2.putText(g, f"{n}s", (5, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
                tiles.append(g)
            n += 1
    cam.release()
    while len(tiles) % 10: tiles.append(np.zeros_like(tiles[0]))
    cv2.imwrite(f"{folder}/sheet.jpg", np.vstack([np.hstack(tiles[k:k + 10]) for k in range(0, len(tiles), 10)]))
    print(f"SAVED {folder}", flush=True)

logf = open(f"{OUT}/game.csv", "a", newline="", buffering=1)   # line-buffered: readable while running
w = csv.writer(logf)
last_fuel = None; filming = None; errors = 0
print("logging", flush=True)
while True:
    try:
        fp, fl, ign, eng, spd, gear, paused = sample(); errors = 0
    except Exception as e:
        errors += 1
        if errors == 10: print(f"ERROR SimHub API: {e}", flush=True)
        time.sleep(1); continue
    w.writerow([time.strftime("%H:%M:%S"), round(fp or 0, 2), round(fl or 0, 1), ign, eng,
                round(spd or 0, 1), gear, paused])
    if fp is not None and last_fuel is not None and fp > last_fuel + 0.05 and (spd or 0) < 1 \
            and not (filming and filming.is_alive()):
        folder = f"{OUT}/refuel_{time.strftime('%H%M%S')}"
        print(f"REFUEL started at {fp:.1f} % (ign {ign}, engine {eng}) -> {folder}", flush=True)
        filming = threading.Thread(target=film, args=(folder,), daemon=True); filming.start()
    last_fuel = fp
    time.sleep(0.2)
