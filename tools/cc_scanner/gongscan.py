"""Listen for cluster sounds while switching check-control codes on one at a time.
usage: python gongscan.py CODES_FILE OUTDIR
CODES_FILE: one code per line. Every code: ON, record 4 s (Brio mic), OFF, 4 s pause.
Codes louder than THRESH get a wav in OUTDIR and a 'SOUND' line; all go to OUTDIR/log.csv."""
import sys, os, time, csv, wave, numpy as np, sounddevice as sd
from scanlib import Rig
MIC = 4; THRESH = 0.06          # turn-signal ticks peaked at 0.41, quiet room ~0.01-0.02
codes = [int(l) for l in open(sys.argv[1]) if l.strip()]
OUT = sys.argv[2]; os.makedirs(OUT, exist_ok=True)
r = Rig()
for c in ("v ign 1", "v light 1", "v fuel 80", "v speed 0"): r.cmd(c)
time.sleep(10)
log = csv.writer(open(f"{OUT}/log.csv", "a", newline="", buffering=1))
try:
    for i, code in enumerate(codes):
        r.cmd(f"c {code}")
        a = sd.rec(int(4 * 44100), samplerate=44100, channels=1, device=MIC, dtype="float32"); sd.wait(); a = a[:, 0]
        r.cmd("x")
        pk = float(np.abs(a).max())
        log.writerow([code, round(pk, 4), time.strftime("%H:%M:%S")])
        if pk > THRESH:
            w = wave.open(f"{OUT}/{code:04d}.wav", "wb"); w.setnchannels(1); w.setsampwidth(2); w.setframerate(44100)
            w.writeframes((np.clip(a, -1, 1) * 32767).astype("<i2").tobytes()); w.close()
            print(f"SOUND code {code} peak {pk:.3f}", flush=True)
        if i % 50 == 0: print(f"progress {i}/{len(codes)} code {code}", flush=True)
        time.sleep(4)
    print("DONE", flush=True)
except Exception as e:
    print("ERROR", type(e).__name__, e, flush=True); raise
finally:
    try: r.cmd("x")
    except Exception: pass
    r.close()
