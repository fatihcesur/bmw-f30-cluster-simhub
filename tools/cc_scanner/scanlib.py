"""Helpers for driving the cc_scanner firmware and grabbing webcam frames."""
import time, cv2, serial
import numpy as np  # noqa: F401
import serial.tools.list_ports

def find_port():
    """The CH340 COM number drifts after replugs (COM8, 19, 25, 30...)."""
    ports = [p.device for p in serial.tools.list_ports.comports() if "CH340" in p.description]
    if not ports:
        raise RuntimeError("no CH340 serial port found")
    return ports[0]

CROP = (slice(100, 800), slice(200, 1700))       # cluster area in the 1920x1080 frame
PIX_THRESHOLD = 250                              # changed pixels needed to call it a hit

# MID info line that rotates (date / avg speed / ...) on every MENU press: ignore it.
# Check-control popups replace the whole MID, so they are still detected elsewhere.
IGNORE = (slice(470, 570), slice(580, 820))      # in CROP coordinates

def diff_score(a, b):
    ga = cv2.GaussianBlur(cv2.cvtColor(a, cv2.COLOR_BGR2GRAY), (5, 5), 0)
    gb = cv2.GaussianBlur(cv2.cvtColor(b, cv2.COLOR_BGR2GRAY), (5, 5), 0)
    mask = cv2.absdiff(ga, gb) > 45
    mask[IGNORE] = False
    return int(mask.sum()), mask

CAM_INDEX = 1  # Creative Live! Cam aimed at the cluster (re-probe if replugged)

class Rig:
    def __init__(self, port=None, cam=CAM_INDEX):
        self.ser = serial.Serial(port or find_port(), 19200, timeout=0.2, write_timeout=3)
        time.sleep(2.5)                      # Nano resets on open
        self.ser.reset_input_buffer()
        self.cam = cv2.VideoCapture(cam, cv2.CAP_DSHOW)
        self.cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
        self.cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
        # auto exposure re-balanced the whole picture whenever the MID changed,
        # which showed up as fake differences on every dial marking: lock it
        self.cam.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0.25)
        self.cam.set(cv2.CAP_PROP_EXPOSURE, -5)
        self.cam.set(cv2.CAP_PROP_AUTO_WB, 0)
        for _ in range(20): self.cam.read()

    def cmd(self, line):
        self.ser.write((line + "\n").encode())
        t = time.time(); out = b""
        while time.time() - t < 1.0:
            out += self.ser.read(200)
            if b"OK" in out or b"ERR" in out: break
        return out.decode(errors="replace")

    def frame(self):
        for _ in range(5): self.cam.read()   # flush buffered frames
        ok, f = self.cam.read()
        if not ok:
            raise RuntimeError("camera: no frame")
        return f

    def close(self):
        self.cam.release(); self.ser.close()
