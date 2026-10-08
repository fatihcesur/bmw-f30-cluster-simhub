"""Pretend to be SimHub: send Custom Serial Device packets to the REAL firmware.

Framing (ArqSerial.h): 0x01 0x01 <id> <len 1..32> <data> <crc8(id,len,data)>, ids 0..128
then wrap to 0. The stream inside is 0x03 'P' + "<54 ;-separated fields>\\n".
usage: python fakesimhub.py  (edit STEPS below)
"""
import time, sys, serial
from scanlib import find_port

CRC = [0,213,127,170,254,43,129,84,41,252,86,131,215,2,168,125,82,135,45,248,172,121,211,6,123,174,4,209,133,80,250,47,164,113,219,14,90,143,37,240,141,88,242,39,115,166,12,217,246,35,137,92,8,221,119,162,223,10,160,117,33,244,94,139,157,72,226,55,99,182,28,201,180,97,203,30,74,159,53,224,207,26,176,101,49,228,78,155,230,51,153,76,24,205,103,178,57,236,70,147,199,18,184,109,16,197,111,186,238,59,145,68,107,190,20,193,149,64,234,63,66,151,61,232,188,105,195,22,239,58,144,69,17,196,110,187,198,19,185,108,56,237,71,146,189,104,194,23,67,150,60,233,148,65,235,62,106,191,21,192,75,158,52,225,181,96,202,31,98,183,29,200,156,73,227,54,25,204,102,179,231,50,152,77,48,229,79,154,206,27,177,100,114,167,13,216,140,89,243,38,91,142,36,241,165,112,218,15,32,245,95,138,222,11,161,116,9,220,118,163,247,34,136,93,214,3,169,124,40,253,87,130,255,42,128,85,1,212,126,171,132,81,251,46,122,175,5,208,173,120,210,7,83,134,44,249]

class FakeSimHub:
    def __init__(self):
        self.s = serial.Serial(find_port(), 19200, timeout=0.05, write_timeout=2)
        time.sleep(2.5)                     # Nano reset
        self.pid = 255
        self.rx = b""

    def _packet(self, data):
        self.pid = 0 if self.pid > 127 else self.pid + 1
        crc = 0
        for b in (self.pid, len(data), *data):
            crc = CRC[crc ^ b]
        self.s.write(bytes([1, 1, self.pid, len(data)]) + data + bytes([crc]))
        time.sleep(0.04); self.rx += self.s.read(256)   # acks + replies, let the firmware consume

    def send(self, fields):
        msg = b"\x03P" + ";".join(str(f) for f in fields).encode() + b"\n"
        for i in range(0, len(msg), 16):       # small chunks: the firmware's ring buffer is 32 bytes
            self._packet(msg[i:i + 16])

def fields(speed=0, rpm=700, fuel=81, ign=1, cons=0, warn=0, light=1, hand=0, gear="N"):
    t = time.localtime()
    f = [0] * 54
    f[0], f[1], f[2], f[3] = speed, rpm, 135, fuel
    f[5] = t.tm_year; f[15] = t.tm_min; f[17] = t.tm_sec; f[19] = t.tm_mday; f[21] = t.tm_mon
    f[20] = 120                      # air pressure
    f[22] = hand                     # parking brake
    f[26] = ign; f[32] = ign         # engine_fs / EngineIgnitionOn
    f[28] = gear; f[29] = light; f[31] = 20; f[33] = round(fuel * 52) if cons == 0 else cons; f[34] = t.tm_hour   # f[33]: fuel left in a 52 L tank, cL
    f[37] = 90; f[44] = warn; f[53] = "ETS2"
    return f

if __name__ == "__main__":
    steps = [(float(a.split(":")[0]), dict(kv.split("=") for kv in a.split(":")[1].split(","))) for a in sys.argv[1:]]
    sh = FakeSimHub()
    for secs, kw in steps:
        kw = {k: (v if k == "x" else int(v)) for k, v in kw.items()}
        print(time.strftime("%H:%M:%S"), secs, "s", kw, flush=True)
        t0 = time.time()
        while time.time() - t0 < secs:
            sh.send(fields(**kw)); time.sleep(0.1)
