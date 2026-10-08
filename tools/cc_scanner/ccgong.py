"""Does a check-control code make the cluster play a gong? One code at a time, Brio mic.
usage: python ccgong.py CODE [CODE ...]"""
import sys, time, numpy as np, sounddevice as sd
from scanlib import Rig
MIC = 4
r = Rig()
for c in ("v ign 1", "v light 1", "v fuel 80", "v speed 0"): r.cmd(c)
time.sleep(10)
def peak(secs):
    a = sd.rec(int(secs * 44100), samplerate=44100, channels=1, device=MIC, dtype="float32"); sd.wait()
    return float(np.abs(a).max())
base = peak(3)
print(f"background peak {base:.4f}", flush=True)
for code in [int(x) for x in sys.argv[1:]]:
    r.cmd(f"c {code}")
    p = peak(4)
    r.cmd("x"); time.sleep(4)
    print(f"code {code:4d}: peak {p:.4f}  {'SOUND' if p > max(0.05, base * 4) else ''}", flush=True)
r.close()
