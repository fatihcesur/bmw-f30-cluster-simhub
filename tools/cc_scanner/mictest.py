"""Does the Brio microphone hear the cluster? Turn signal off vs on (the cluster ticks
through its own speaker). usage: python mictest.py [mic_index]"""
import sys, time, threading, numpy as np, sounddevice as sd
from fakesimhub import FakeSimHub, fields
MIC = int(sys.argv[1]) if len(sys.argv) > 1 else 4
sh = FakeSimHub(); time.sleep(8)
sig = [0]; stop = [False]
def feed():
    while not stop[0]:
        v = fields(fuel=80, ign=1); v[13] = sig[0]; v[23] = sig[0]
        sh.send(v); time.sleep(0.1)
threading.Thread(target=feed, daemon=True).start()
def rec(secs=5):
    a = sd.rec(int(secs * 44100), samplerate=44100, channels=1, device=MIC, dtype="float32"); sd.wait()
    a = a[:, 0]
    fr = np.abs(a[:len(a) // 441 * 441]).reshape(-1, 441).max(axis=1)     # 10 ms peaks
    return a, fr
time.sleep(10)                       # past the 7 s ignition-on wake
_, off = rec(); sig[0] = 1; time.sleep(1.5); a, on = rec(); sig[0] = 0; time.sleep(1)
stop[0] = True
print(f"signal OFF: median {np.median(off):.4f}  p99 {np.percentile(off,99):.4f}  max {off.max():.4f}")
print(f"signal ON : median {np.median(on):.4f}  p99 {np.percentile(on,99):.4f}  max {on.max():.4f}")
thr = np.percentile(off, 99.5) * 2
ticks = np.flatnonzero((on[1:] > thr) & (on[:-1] <= thr))
print("transients above 2x background:", len(ticks), "at s:", [round(t / 100, 2) for t in ticks[:20]])
import wave
w = wave.open("captures/mic_signal_on.wav", "wb"); w.setnchannels(1); w.setsampwidth(2); w.setframerate(44100)
w.writeframes((np.clip(a, -1, 1) * 32767).astype("<i2").tobytes()); w.close()
