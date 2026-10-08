"""Photograph the cluster every N seconds while the user drives (camera only; SimHub owns the serial port)."""
import sys, time, os, cv2
from scanlib import CAM_INDEX, CROP
every = float(sys.argv[1]) if len(sys.argv) > 1 else 20
c = cv2.VideoCapture(CAM_INDEX, cv2.CAP_DSHOW); c.set(3, 1920); c.set(4, 1080)
c.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0.25); c.set(cv2.CAP_PROP_EXPOSURE, -5); c.set(cv2.CAP_PROP_AUTO_WB, 0)
while True:
    for _ in range(5): c.read()
    ok, f = c.read()
    if ok: cv2.imwrite(f"captures/ingame/{time.strftime('%H%M%S')}.jpg", f[CROP], [cv2.IMWRITE_JPEG_QUALITY, 85])
    print(time.strftime('%H:%M:%S'), "ok" if ok else "NO FRAME", flush=True)
    time.sleep(every)
