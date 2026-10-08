# BMW F30 cluster — SimHub project notes

Read automatically at session start. This is the short version: rules, pin map,
current state. The full dated investigation log (every experiment, dead end and
why) lives in `docs/HISTORY.md` — grep it for a CAN ID or feature name before
re-investigating anything. Research/candidate lists for unsolved items live in
`docs/RESEARCH.md`.

## Arduino Nano pin usage (check before wiring anything new)

| Pin | Used for | Notes |
|-----|----------|-------|
| D2  | MCP2515 INT | `intPin` in `SHCustomProtocol.h` |
| D3  | Button 1 slot | Claimed by `SHButton::begin()`, nothing wired. Button 1 is really the cluster's own trip button, read over CAN `0x5E0` |
| D4  | Button 2 – MID page / long-press trip reset | Sends `0x1EE` (steering wheel MENU) while held |
| D5  | Button 3 – drive mode cycle | `drivemod` on `0x3A7`, cycles `{1,2,4,5,6,7}` |
| D6  | Button 4 – consumption unit cycle | `setmpgl100orkml` 1=l/100km 2=mpg 3=km/l |
| D7  | Button 5 – real seatbelt buckle switch | Level, not press. Drives `0x5c0` code 77 only |
| D8, D9, A0–A7 | Free | Next GPIO input starts at D8 |
| D10 | MCP2515 CS | `spiCS` |
| D11–D13 | Hardware SPI | Never reuse |

Pin defaults live in `bmw_f30_cluster.ino` near `#define ENABLED_BUTTONS_COUNT`.

MCP2515 module → Nano (original wiring, what the firmware expects): VCC→5V, GND→GND,
CS→D10, SO→D12, SI→D11, SCK→D13, INT→D2. Moving SO/SI/SCK/VCC/GND to the ICSP header
also works without code changes (same SPI pins), but CS and INT must stay on D10/D2.

"Cluster dead" check (2026-10-08): flash `tools/bench/alltest/alltest` (standalone, no SimHub,
all lamps on, 100 km/h / 5000 rpm, prints `ok/err/TEC/REC` at 115200). The MCP2515 still
answers "OK" over SPI with its VCC unpowered (back-fed through the SPI pins), but the
TJA1050 then can't work: every send fails with TEC=0 REC≈128. Healthy alone (no bus):
TEC=128 REC=0. Healthy on the cluster: ok rising, err=0. That day VCC was on the Nano's
RST pin (next to 5V), which also broke auto-reset/upload.
Second fault the same day: with the module fixed, plugging in the USB-C extension pulled
CAN-L to ~1.5 V (H 2.5 V) = bus stuck dominant, TEC=0 REC≈128, no frames from the cluster.
The user traced it to bad solder joints on the USB-C breakout sockets (being redone).

## Cluster ↔ Arduino extension cable (USB-C used as a 4-wire connector, 2026-09-29)

A USB-C C-to-C cable with 4-pin female breakout boards (V, G, D+, D−) at both ends
carries power and CAN — it is NOT a USB connection:

| Signal | Breakout pin (both ends identical) |
|---|---|
| +12 V (cluster supply) | V (VBUS) |
| GND (cluster GND = Arduino GND) | G |
| CAN-H | **D−** |
| CAN-L | **D+** |

- D−=CAN-H / D+=CAN-L is deliberate; it only has to match on both ends.
- Works in both plug orientations with breakouts that tie A6/B6 and A7/B7 (check
  with a multimeter); needs a data-capable cable (charge-only cables lack D+/D−).
- Continuity check: CAN-H↔CAN-H beeps, CAN-H↔CAN-L doesn't, 12V↔GND doesn't; repeat
  with the plug flipped.
- **Never plug a phone/PC into these sockets**: VBUS carries a raw 12 V. Label them.
- No spare wires: extra buttons can't use this cable. Do NOT put buttons on CAN-H/L
  (pressing = bus short = comms stop). Use steering-wheel buttons via SimHub
  `InputStatus.JoystickPlugin.*` fields instead, or a different connector.
- Reverse-polarity protection on the 12 V input: 1N4007 in series (anode = supply +,
  cathode/stripe = cluster +12 V, ~0.8 V drop, cluster draws ~0.5 A); nothing on GND.

## Hardware / toolchain

- Real BMW F30 KOMBI (not a 6WA), Arduino Nano + MCP2515 (**8 MHz crystal**:
  `CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)`), SimHub over USB serial 19200 baud.
  Arduino on USB power; only GND shared with the cluster's own 12V.
- Build/flash: `C:\Users\fatih\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe`,
  FQBN `arduino:avr:nano`. COM port drifts (COM8 → COM19 → COM25): check
  `Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match 'CH340' }` each time.
  `cannot set com-state` = CH340 glitch → unplug/replug USB.
- **SimHub must be closed before upload. Ask the user to close it from its own UI**
  — `Stop-Process -Force` skips its auto-persist and silently discards an unsaved
  Custom Serial Device formula (cost a whole session of wrong debugging once).
- `#define INCLUDE_BUTTONS` is added manually in `bmw_f30_cluster.ino`; any other
  `INCLUDE_*`-gated SimHub peripheral needs the same (SimHub's generator never runs here).

## Reference first

The reference project's `BMWFSeriesCluster.cpp` (linked in the README credits)
lists F30 6WA as supported (ours is not a 6WA, see below). Check it for a CAN ID before any trial and error, but
**treat it as hints, not truth**: the user confirmed (2026-09-28) the reference project's cluster is a
different series from ours, and our working `0x289` payload already differs from it.
Frames the cluster itself transmits (sniffed 2026-09-28): 660 1B3 328 362 336 338 205
2C5 2F7 2CA 560 35C 393 5E0 367 330. No 0x2F8, so it does not broadcast the clock.
It does NOT have: ambient temp, clock/date, cruise set speed, avg consumption/range.

## customprotocol ↔ SimHub rules

- Game-aware since 2026-10-08: truck fields are guarded by `CurrentGame=='ETS2' or 'ATS'`;
  the else branches use BeamNG raw `light_*` OR SimHub common data (`TurnIndicatorLeft/Right`,
  `Handbrake` > 50, `PitLimiterOn` -> cruise icon) OR ACC `Graphics.LightsStage`. Field 54
  sends `CurrentGame` (unused by the firmware). Assumes those common fields are numeric;
  **not yet verified in Assetto Corsa** - read them from `localhost:8888/api/getgamedata`
  (`NewData`) while AC runs. Pre-AC ETS2-only formula: `simhub/legacy/ets2_formula_2026-10-08.txt`.
- Exactly **54 fields**, same order as `SHCustomProtocol::read()`. Many fields are
  repurposed dead variables (name ≠ meaning): field 3 `Temp`=oil temp×1.5,
  field 21 `acc_lightstage`=brake air pressure, field 36 `braketemp`=wheel B02
  held, field 52 `dmode`=drive-mode button (`Maple_B10`), field 34 `throthel`=
  consumption (parsed but **not sent anywhere**). Before repurposing another field,
  grep the variable — only reuse if it's just declaration + parse.
- Repo file is truth **only if synced**. The user has edited in SimHub directly
  several times. If behavior doesn't match this file, first ask the user to paste
  back the live formula. After a paste, confirm SimHub was closed cleanly/saved.
- SimHub formulas: `if(c,a,b)` (not `iif`); booleans arrive as `"True"`, so always
  `if([X]==true,1,0)`; wrap numerics in `isnull([X],0)`; joystick props need the
  `InputStatus.` prefix (`[InputStatus.JoystickPlugin.R3_Racing_Wheel_and_Pedals_B02]`)
  — use the editor's "Insert property" button rather than hand-typing.
- Verify an ETS2 property exists in SimHub's Available Properties while driving
  before trusting it; wrong names silently read 0.
- After an `Edit` on `simhub/custom_protocol.txt`, re-read it: many identical `'0' + ';' +`
  lines make it easy to hit the wrong field.

## Cluster behavior rules (learned the hard way)

- **Arduino reflash ≠ cluster reset.** Lamps/backlight/gauges can stay stuck at a
  stale state; do a real 12V power-cycle of the cluster before blaming code.
- **`0x5c0` check-control codes: test ONE code at a time.** Sending several codes in
  a burst confused the parser (airbag lamp flashed, needed 12V cycle).
  Known codes: 24 park brake yellow (used for air pressure < 80), 34 check-engine
  (shows as triangle-!, not the engine-shaped MIL), 35/215 DSC, 36 DSC off,
  71 park brake red (hardcoded off — was mislabeled "seatbelt"), 77 seatbelt.
- **Guessed CAN IDs from other generations are not harmless.** One-at-a-time only,
  and suspect the newest guessed ID first when something else breaks.
- The cluster silently ignores out-of-range or unrecognized values (no errors).
- `0x2BB` distance counter is an accumulator: must stay 100 ms time-gated. Multiplier
  is `Speed*2.9` (correct). Odometer inflation does not matter: the user confirmed
  (2026-09-28) the cluster will never go back into a car, so fake speed is fine in tests.
- A message that stops arriving is not "off": the cluster holds the last value, so
  always send an explicit off state (backlight `0x202` lesson).
- Fuel needle is heavily damped inside the cluster (minutes to settle). Data path is
  proven instant. Don't reopen this (see HISTORY: fuel-sweep saga v1–v5).
- Headlights on flips display theme white→orange; tied to `0x21A`, not separable.
- Clock is the cluster's own RTC set via its menu, not CAN-fed (F30 forum).
  `0x193`/`0x2F8` (E90 DBC) did nothing and are commented out.
- F-series checksum: CRC-8/SAE-J1850 (poly 0x1D, init 0x00) with a per-message
  XOR-out that must be recovered from real captures (CrcBeagle).

## Current state (end of 2026-09-23 session)

Working: speed, RPM (0xF3 fixed 2026-09-28: 0x60|counter + CRC 0x7A, rpm*1.557/256 in
byte2, tested 0-7000 on camera; the formula still has RPM ×1.2, review with the user), fuel, oil temp, gear PRND/DS, turn signals + hazards, high beam, cruise
on/off icon, parking brake (`0x34F`), seatbelt (77), drive mode, consumption unit,
MID page + long-press trip reset (D4 and wheel B02), tach OFF/READY, backlight always on (`backlightAlwaysOn`, 2026-10-08; was tied
to headlights), welcome sweep, Turkish language (`setlanguage = 0`), range bar
CRC fix (`0x2C4`, poly 0xC6).

Open / unconfirmed:
- Brake air pressure → yellow lamp (code 24, threshold 80): re-confirm live first.
- Low-beam icon: SOLVED as "doesn't exist" — full `0x21A` sweep (2026-09-28) shows the
  green icon is the only lights-on indicator, and bit 0x04 also switches the MID to the
  red night theme. `nightThemeWithLights = false` (default, user wants white text)
  masks that bit. Not yet flashed/confirmed with SimHub running.
- Engine-shaped MIL icon: `0x5c0` code **34** (confirmed on camera 2026-09-28).
  ~560 check-control codes with photos: `docs/RESEARCH.md`, `tools/cc_scanner/`.
- Clock/date: SOLVED with `0x39E` (set time), sent from SimHub PC time. Flashed
  2026-09-28 morning; confirm with SimHub running (user must re-paste `simhub/custom_protocol.txt`,
  line 5/6 swap fixed the year field).
- Cruise set speed: not in any 0x289 byte (swept at 80 km/h). Needs sniffing.
- Average consumption / range: SOLVED on the bench (2026-09-28): 0x2C4 byte0 is the
  fuel counter, factor 114 calibrated; field 34 = ETS2 consumption scaled to a 57 L
  car tank. Needs in-game confirmation (capacity property name, 57 L guess).
- Fuel gauge: standstill ~30 s full sweep, driving ~2°/min; ≥6 s CAN silence makes it
  jump. Firmware (2026-10-04): 7 s silence on ignition-on and once the refuelled level
  has been steady 2.5 s (a wake while the level is still rising does nothing), and the
  silence ends at once when Speed > 2. Verified with `tools/cc_scanner/refueltest.py`.
  ETS2 refuels with the ignition off, and with the ignition off the cluster ignores the
  level; so while stopped and the fuel rises, 0x12F says "ignition on" (`refuelIgnition`)
  until the real ignition comes on or the truck moves (max 2 min). The needle follows the
  fill live and is full when the player drives off 1 s after ignition-on (refuel8).
- In-game (SimHub API, 2026-10-04): ETS2 keeps the IGNITION ON while refuelling (engine
  off) and fills in ~10 s, then the player drives off at once. Fix (REFUEL_MODE 1, bench
  rmode1c): CAN silence from the first fuel rise until 1 s after the last rise (min 7 s),
  so the cluster wakes on the final level, then 0x1A1 reports 0 km/h for 5 s so the needle
  finishes its jump before the slow driving filter starts. Sending 100 % during the fill
  (REFUEL_MODE bit1) froze the needle at empty - never jump 0x349 by a big step.
- ROOT CAUSE of the slow/odd fuel needle (2026-10-04): the cluster fuses the 0x349 sender
  with its own consumption integral (0x2C4 counter x 0x2BB distance). The counter came from
  ETS2 consumption x speed and never matched the sender, so the cluster learned corrections
  (0x330 showed 5 L at 99 %) and distrusted refuels. Now field 34 = fuel left in a 52 L tank
  in cL (FuelPercent x 52) and the counter follows the real decrease (41 counts per cL).
  Two consistent drive+refuel rounds un-learned it: 0x330 = 52 L at 99 %, needle full.
  Read the cluster's own estimate with `tools/cc_scanner/read330.py` (scanner firmware;
  0x330 only comes right after a wake). Never run fake drives with fuel that doesn't match.
- P: ETS2 has no park position; N + parking brake + standing still shows P (0x3FD 0x20).
- Drive mode starts in Eco Pro (`drivemod = 7`).
  Never send implausible 0x349 values (latches the gauge).
- Never stop the Arduino/CAN while the (fake) speed is > 0: latches the l/100km
  needle at 20 until a long silence. Ambient temperature is a physical NTC, not CAN.
- Fog light (`AuxFront`) never verified live. Field 38 `WaterTemperature` path broken
  for ETS2 (overheat lamp likely never fires).

## ETS2 warnings (field 45 bitmask, added 2026-09-28)

bit0 any dmg >= 15 % -> MIL 34; bit1 any dmg >= 20 % -> 29 Tahrik; bit2 any >= 40 %
-> 41 Servis; bit3 engine >= 50 % -> 170 Motor arızalı (10 s popups); bit4 diff lock
-> 780; bit5 (speed > limit + 5) no longer shown - instead speed > 100 km/h -> 78 yellow
"! Hiz uyarisi" and > 120 -> 62 red, 3 s popups each (user request 2026-10-04); bit6 trailer attach/detach -> 858 / 75;
bit7 average wheel wear >= 25 % -> 265 "Lastik basincini kontrol edin" (10 s). Fuel < 15 % -> 275
"Yakit rezervi" (10 s, firmware-side). ETS2 has no per-wheel tyre data.
One code ON at a time. Verified property names (user, SimHub): DamageValues.Engine /
.Chassis (0-1 scale), TruckValues.CurrentValues.DifferentialLock,
NavigationValues.SpeedLimit.Kph, TrailerValues01.Attached. Transmission/Cabin/WheelsAvg
not confirmed (isnull -> 0). Not yet tested in game.

## RAM budget

The Nano is at ~57 % RAM (880 bytes free since 2026-10-08: own bitwise `CRC8.h` replaced
the reference project's table-based CRC8.cpp, which kept a 256-byte table in RAM). Below ~500 bytes free, SimHub detection fails with
"Invalid version" (see docs/RESEARCH.md). Check the compile output after every change;
use F() for literal strings and avoid new global arrays.

## Git

`origin` = **public** repo `https://github.com/fatihcesur/bmw-f30-cluster-simhub` (fresh
history from 2026-10-08, MIT). Everything pushed there is public: no VIN, serial numbers,
personal e-mail or private photos. Commits use the GitHub noreply address (repo-local
`user.email`). `archive` = private `fatihcesur/bmw-f30-cluster-simhub-archive` with the full
pre-2026-10-08 history (tag `ets2` = early known-good ETS2 state); local branch
`archive/full-history` has the same. `upstream` = original upstream project (read-only).
GitHub CLI: `C:\Program Files\GitHub CLI\gh.exe`, logged in as `fatihcesur`.
`tools/cc_scanner/captures/` (~3.5 GB raw frames) stays out of git; copy chosen frames
into `docs/` instead.
