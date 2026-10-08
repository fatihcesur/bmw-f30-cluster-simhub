"""Log what the cluster itself transmits on a few ids (scanner firmware "s <id>").

usage: python kombitx.py SECONDS ID [ID ...]   e.g. python kombitx.py 20 1B3 35C 362 367
Prints, per id, how many frames arrived and the distinct payloads (count each),
so constant status frames and changing counters are easy to tell apart.
"""
import sys, time, collections
from scanlib import Rig

secs = float(sys.argv[1])
r = Rig()
for c in ("v ign 1", "v light 1", "v fuel 80", "v speed 0"):
    r.cmd(c)
time.sleep(5)
for hexid in sys.argv[2:]:
    r.cmd(f"s {hexid}")
    seen = collections.Counter(); buf = b""; t0 = time.time()
    while time.time() - t0 < secs:
        buf += r.ser.read(256)
        *lines, buf = buf.split(b"\n")
        for ln in lines:
            ln = ln.decode(errors="replace").strip()
            if ln.startswith("RX "):
                seen[" ".join(ln.split()[2:])] += 1
    print(f"0x{hexid}: {sum(seen.values())} frames, {len(seen)} distinct payloads")
    for p, n in seen.most_common(8):
        print(f"   {n:4d}x  {p}")
r.cmd("s -1")
r.close()
