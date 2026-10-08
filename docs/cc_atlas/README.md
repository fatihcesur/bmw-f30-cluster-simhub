# Check-control code atlas (BMW F30 KOMBI)

Every check-control code `0..949` was sent to this cluster one at a time on CAN `0x5C0`
(`{0x40, code, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF}` = on, byte 3 `0x28` = off) and
photographed with a webcam. Menu language: Turkish. Scanner: `tools/cc_scanner/scan.py`
with the `tools/cc_scanner/cc_scanner` firmware.

- `sheets/`: contact sheets, 50 codes per page, only codes that changed the display.
- `popups/NNNNN.jpg`: zoomed MID crop for each code that showed something (581 codes).
- `codes_with_visible_change.txt`: the list of those codes.

Readable descriptions of most codes (Turkish text, lamp colour, notes) are in
`docs/RESEARCH.md`.

Codes worth knowing: 24 yellow parking-brake lamp, 34 engine MIL, 35/215 DSC, 36 DSC off,
62 / 78 speed warning (red / yellow), 71 red brake lamp, 77 seatbelt, 275 fuel reserve,
521–530 speed-limit info, 706 LIM.

Send one code at a time: several codes in a burst confused the cluster once (airbag lamp,
needed a 12 V power-cycle). Photos were taken at an angle with the original amber
backlight, so some needles look off-scale.
