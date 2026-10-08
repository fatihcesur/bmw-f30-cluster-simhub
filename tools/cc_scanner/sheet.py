"""usage: python sheet.py START END  -> captures/sheets/sheet_START_END.jpg of zoomed hits (3 per row)"""
import sys, glob, os, cv2, numpy as np
a, b = int(sys.argv[1]), int(sys.argv[2])
fs = [f for f in sorted(glob.glob("captures/mid/*.jpg")) if a <= int(os.path.basename(f)[:5]) <= b]
if not fs: print("no hits"); sys.exit()
ims = [cv2.resize(cv2.imread(f), (560, 210)) for f in fs]
while len(ims) % 3: ims.append(np.zeros_like(ims[0]))
os.makedirs("captures/sheets", exist_ok=True)
out = f"captures/sheets/sheet_{a:04d}_{b:04d}.jpg"
cv2.imwrite(out, np.vstack([np.hstack(ims[i:i+3]) for i in range(0, len(ims), 3)]))
print(out, len(fs), "hits")
