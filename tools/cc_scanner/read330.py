"""Read the cluster's own fuel estimate (0x330) with the scanner firmware, standing still.
usage: python read330.py FUEL_PERCENT [seconds]"""
import sys, time
from scanlib import Rig
fuel = int(sys.argv[1]); secs = float(sys.argv[2]) if len(sys.argv) > 2 else 20
r = Rig()
for c in ("v light 1", "v speed 0", "v cons 0", f"v fuel {fuel}", "v ign 1", "s 330"): r.cmd(c)
r.cmd("v mute 9000")     # 0x330 only comes right after the cluster wakes: let it sleep first
buf = b""; t = time.time(); last = None
while time.time() - t < secs:
    buf += r.ser.read(256); *ls, buf = buf.split(b"\n")
    for l in ls:
        if l.startswith(b"RX 330"):
            b = [int(x, 16) for x in l.split()[2:]]
            print(f"{time.time()-t:5.1f}s  litres {b[3]}  sender bytes {b[4]:02X} {b[5]:02X}  range {(b[6] | b[7] << 8)/16} km", flush=True)
r.cmd("s -1"); r.close()
