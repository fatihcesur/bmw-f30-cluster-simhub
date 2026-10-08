"""Log what the cluster itself broadcasts while we drive a fake scenario.
usage: python sniff.py SECONDS "cmd1;cmd2;..."   -> prints 0x330 range and 0x2C5 bytes over time"""
import sys, time
from scanlib import Rig

secs = float(sys.argv[1]); cmds = [c for c in sys.argv[2].split(";") if c] if len(sys.argv) > 2 else []
r = Rig(); time.sleep(4)
for c in cmds: r.cmd(c)
def grab(cid, t):
    r.ser.reset_input_buffer(); r.ser.write(f"s {cid}\n".encode())
    t0 = time.time(); buf = b""
    while time.time() - t0 < t:
        buf += r.ser.read(300)
        if b"RX" in buf and buf.endswith(b"\n"): break
    return [l for l in buf.decode(errors="replace").splitlines() if l.startswith("RX")]
t0 = time.time()
while time.time() - t0 < secs:
    a = grab("330", 8); b = grab("2C5", 1)
    rng = None
    if a:
        p = a[-1].split()[2:]
        if len(p) >= 8: rng = (int(p[7], 16) << 8 | int(p[6], 16)) / 16
    print(f"{time.time()-t0:5.0f}s range={rng} 330={a[-1][7:] if a else '-'} 2C5={b[-1][7:] if b else '-'}", flush=True)
r.ser.write(b"s -1\n"); r.cmd("v speed 0"); r.cmd("v rpm 0"); r.close()
