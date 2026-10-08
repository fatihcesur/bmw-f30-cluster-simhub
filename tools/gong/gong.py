"""BMW-style warning gong from the PC speakers, driven by SimHub's ETS2 data.

The real F30 gong is played by the head unit (door speakers), not by the KOMBI, so the
bench cluster can't make it. This reads SimHub's web API (localhost:8888) and plays a
two-tone chime when the cluster shows a warning, with the same rules as the firmware
(firmware/bmw_f30_cluster/SHCustomProtocol.h):
  fuel  < 15 %             -> chime ("Yakit rezervi");               re-arms above 20 %
  any damage >= 20 % / 40 % -> chime ("Tahrik" / "Servis gerekli" popups); each once,
                              re-arms after a repair (back below the level)
No chime for the speed warnings (user request 2026-10-04).
Damage = SimHub's CarDamagesMax (largest of the truck's damage values, 0..100).
usage: python gong.py [--test] [--volume 0.5] [--device N] [--sound file.wav]
Put the real BMW chime as tools/gong/chime.wav (WAV) to use it instead of the built-in tone.
Runs until closed. --test plays the chime once and exits.
"""
import sys, time, json, urllib.request
import numpy as np, sounddevice as sd

URL = "http://localhost:8888/api/getgamedata"
RATE = 44100

def arg(name, default):
    return type(default)(sys.argv[sys.argv.index(name) + 1]) if name in sys.argv else default

VOLUME = arg("--volume", 0.5)
DEVICE = arg("--device", -1)

def chime():
    """Two soft bell tones (~760 Hz then ~620 Hz, measured from the cluster) with
    harmonics and an exponential decay - close to BMW's ding-dong."""
    out = []
    for f, dur in ((760, 0.55), (620, 1.1)):
        t = np.arange(int(RATE * dur)) / RATE
        tone = (np.sin(2 * np.pi * f * t) + 0.35 * np.sin(2 * np.pi * 2 * f * t)
                + 0.15 * np.sin(2 * np.pi * 3.01 * f * t))
        env = np.exp(-t * 4.5) * np.minimum(1, t / 0.005)       # 5 ms attack, bell decay
        out.append(tone * env)
    s = np.concatenate(out)
    return (s / np.abs(s).max() * VOLUME).astype("float32")

def load_wav(path):
    """Use a recorded chime (e.g. the real BMW one) if present: chime.wav next to this file."""
    import wave, os
    if not os.path.exists(path):
        return None
    global RATE
    w = wave.open(path)
    RATE = w.getframerate()
    raw = np.frombuffer(w.readframes(w.getnframes()), {1: "u1", 2: "<i2", 4: "<i4"}[w.getsampwidth()])
    a = raw.astype("float32")
    a = (a - 128) / 128 if w.getsampwidth() == 1 else a / float(2 ** (8 * w.getsampwidth() - 1))
    if w.getnchannels() > 1: a = a.reshape(-1, w.getnchannels()).mean(axis=1)
    return (a / max(1e-6, np.abs(a).max()) * VOLUME).astype("float32")

import os
CHIME = load_wav(arg("--sound", os.path.join(os.path.dirname(os.path.abspath(__file__)), "chime.wav")))
if CHIME is None:
    CHIME = chime()
    print("gong: no chime.wav found, using the built-in ding-dong", flush=True)

def play(times=1):
    for i in range(times):
        sd.play(CHIME, RATE, device=None if DEVICE < 0 else DEVICE)
        sd.wait()

if "--test" in sys.argv:
    play(1); time.sleep(0.3); play(2); sys.exit()

low_fuel = False
dmg_level = 0                 # 0 none, 1 >= 20 %, 2 >= 40 % (already chimed)
errors = 0
print("gong: watching SimHub", flush=True)
while True:
    try:
        d = json.load(urllib.request.urlopen(URL, timeout=2))
        n = d.get("NewData") or {}
        errors = 0
    except Exception:
        errors += 1
        time.sleep(2 if errors > 5 else 0.5)
        continue
    if not n or d.get("GamePaused"):
        time.sleep(0.5); continue
    ign = n.get("EngineIgnitionOn") == 1
    fuel = n.get("FuelPercent") or 0
    if 0 < fuel <= 1: fuel *= 100                             # some games report 0..1
    if ign and not low_fuel and 0 < fuel < 15:
        low_fuel = True; print("gong: fuel reserve", flush=True); play(1)
    if fuel > 20: low_fuel = False
    dmg = n.get("CarDamagesMax") or 0
    if 0 < dmg <= 1: dmg *= 100
    level = 2 if dmg >= 40 else 1 if dmg >= 20 else 0
    if ign and level > dmg_level:
        print(f"gong: damage {dmg:.0f} %", flush=True); play(1)
    dmg_level = level           # rises chime once per level; a repair lowers it again
    time.sleep(0.2)
