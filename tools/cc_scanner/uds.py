"""READ-ONLY UDS requests to the KOMBI over the scanner firmware (BMW F-series style).

BMW extended addressing: tester sends on 0x6F1 with byte0 = ECU address (KOMBI 0x60),
the ECU answers on 0x600 + address (0x660) with byte0 = 0xF1. ISO-TP on top, so a
single frame carries at most 6 data bytes.

usage: python uds.py 22F190 22F101 ...   (hex requests; only services 0x22/0x19/0x1A
       and 0x3E are allowed, so nothing is ever written to the cluster)
"""
import sys, time
from scanlib import Rig

ECU = 0x60
SRC = 0xF1             # tester address: request id 0x600 + SRC, ECU answers to SRC
ALLOWED = {0x22, 0x19, 0x1A, 0x3E}
PAD = 0x55             # ISO-TP padding: frames are always 8 bytes

def frames(r, timeout):
    """Yield (bytes) of every 0x660 frame until timeout seconds of silence."""
    buf = b""; t = time.time()
    while time.time() - t < timeout:
        buf += r.ser.read(256)
        *lines, buf = buf.split(b"\n")
        for ln in lines:
            ln = ln.decode(errors="replace").strip()
            if ln.startswith("RX 660"):
                t = time.time()
                yield bytes(int(x, 16) for x in ln.split()[2:])

def request(r, req):
    assert req[0] in ALLOWED, "refusing anything but read services"
    assert len(req) <= 6
    # write directly: Rig.cmd() would swallow the answer while it waits for "OK"
    # pause our own ~25 frames per Loop for 3 s: with them going out, the MCP2515's two
    # receive buffers overflow and consecutive frames get lost (3 s < the 6 s sleep timeout)
    r.ser.write(b"v mute 3000\n"); time.sleep(0.05)
    r.ser.reset_input_buffer()
    payload = [ECU, len(req), *req, *([PAD] * (6 - len(req)))]
    r.ser.write((f"R {0x600 + SRC:X} " + " ".join(f"{b:02X}" for b in payload) + "\n").encode())
    data, need = b"", None
    for f in frames(r, 4.0):
        if len(f) < 2 or f[0] != SRC:
            continue
        pci = f[1] >> 4
        if pci == 0:                                   # single frame
            data = f[2:2 + (f[1] & 0x0F)]
            if len(data) >= 3 and data[0] == 0x7F and data[2] == 0x78:
                continue                               # response pending, keep waiting
            return data
        if pci == 1:                                   # first frame -> send flow control
            need = ((f[1] & 0x0F) << 8) | f[2]
            data = f[3:]
            r.ser.write(f"R {0x600 + SRC:X} {ECU:02X} 30 00 50 {PAD:02X} {PAD:02X} {PAD:02X} {PAD:02X}\n".encode())
        elif pci == 2 and need:                        # consecutive frame, placed by sequence number
            k = len(data)
            seq = f[1] & 0x0F
            want = ((k - 5) // 6 + 1) & 0x0F               # first frame holds 5 bytes, every CF 6
            if seq != want:
                print(f"   (frame {seq:X} arrived, expected {want:X}: one was lost)")
                return None
            data += f[2:]
            if len(data) >= need:
                return data[:need]
    return data or None

if __name__ == "__main__":
    if "--clear-dtc" in sys.argv:          # ClearDiagnosticInformation: fault memory only, nothing else
        sys.argv.remove("--clear-dtc"); ALLOWED.add(0x14)
    if sys.argv[1].startswith("src="):
        SRC = int(sys.argv.pop(1)[4:], 16)
    r = Rig()
    for c in ("v ign 1", "v light 1", "v fuel 80", "v speed 0", "s 660"):
        r.cmd(c)
    time.sleep(3)
    for h in sys.argv[1:]:
        req = bytes.fromhex(h)
        for attempt in range(6):           # a consecutive frame is sometimes lost: ask again
            resp = request(r, req)
            if resp:
                break
        txt = "" if not resp else "".join(chr(b) if 32 <= b < 127 else "." for b in resp)
        print(f"{h}: {resp.hex(' ') if resp else 'no answer'}   {txt}", flush=True)
    r.cmd("s -1"); r.close()
