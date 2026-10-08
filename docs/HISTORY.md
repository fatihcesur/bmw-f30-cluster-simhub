# BMW F30 cluster — SimHub project notes

This file is read automatically by Claude Code at the start of every session in this
folder. It exists so a future session (fresh or resumed) doesn't have to re-discover
everything the hard way. Read this before touching the code.

## Arduino Nano pin usage (check this before wiring anything new)

| Pin | Used for | Notes |
|-----|----------|-------|
| D2  | MCP2515 CAN shield INT | `intPin` in `SHCustomProtocol.h`, don't reuse |
| D3  | Button 1 slot (unused) | `BUTTON_PIN_1` default, but button 1 is actually the cluster's own trip button reported over CAN (`0x5E0`), not a real GPIO button - this pin is claimed by `SHButton`'s `begin()` call but nothing is physically wired to it |
| D4  | Button 2 - MID/menu page cycle | Momentary pushbutton to GND, `INPUT_PULLUP`. Sends CAN `0x1EE` (steering wheel MENU button) |
| D5  | Button 3 - drive mode cycle | Momentary pushbutton to GND, `INPUT_PULLUP`. Cycles `drivemod` through Traction/Comfort/Sport/Sport+/DSC off/Eco Pro on `0x3A7` |
| D6  | Button 4 - consumption unit cycle | Momentary pushbutton to GND, `INPUT_PULLUP`. Cycles `setmpgl100orkml` (l/100km/mpg/km-l) |
| D7  | Button 5 - real seatbelt buckle switch | Continuous level (not a press event), `INPUT_PULLUP`, switch shorts to GND when buckled. Drives `0x5c0` code 77 only (code 71 is deliberately hardcoded off - see "Real seatbelt buckle switch" note below, it couples to the parking-brake lamp) |
| D8  | Free | Next new GPIO button/switch should start here |
| D9  | Free | |
| D10 | MCP2515 CAN shield CS (chip select) | `spiCS` in `SHCustomProtocol.h`, don't reuse |
| D11 | Hardware SPI MOSI | Used implicitly by the MCP2515 shield's SPI library, fixed by the Nano's hardware, don't reuse |
| D12 | Hardware SPI MISO | Same as above |
| D13 | Hardware SPI SCK | Same as above |
| A0-A7 | Free | Not used by anything currently |

Every `BUTTON_PIN_N` default and its live pin assignment lives in `bmw_f30_cluster.ino`
near `#define ENABLED_BUTTONS_COUNT` - update this table there too if pins change.

## Hardware / setup

- Real BMW F30/F3x instrument cluster (not a 6WA), driven by an Arduino Nano + MCP2515 CAN
  shield, talking to SimHub over USB serial (19200 baud) and to the cluster over CAN
  (500kbps, 8MHz crystal — NOT 16MHz, see `CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)`
  in `SHCustomProtocol.h`).
- Arduino is powered via USB (independent of the cluster's own 12V), only GND is
  shared with the cluster.
- Games used: ETS2 (Euro Truck Simulator 2) primarily, originally built for BeamNG.
- `simhub/custom_protocol.txt` (repo root) is pasted into SimHub's Custom Serial Device editor —
  it's the formula that builds the `;`-separated string sent to the Arduino. It must
  stay at exactly 54 fields, in the same order the Arduino's `read()` function in
  `SHCustomProtocol.h` expects them (check both if changing field count).
- Toolchain used to build/flash: Arduino IDE 2.x's bundled `arduino-cli.exe` at
  `C:\Users\fatih\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe`.
  Board FQBN: `arduino:avr:nano`. Port: was `COM8`, changed to **`COM19`** at some
  point during the 2026-08-28 session (Windows re-enumerated the CH340 after a
  replug/driver reset) - always check `Get-CimInstance Win32_PnPEntity | Where-Object
  { $_.Name -match 'CH340' }` first if upload fails with a port-not-found error,
  don't assume the port number is fixed. SimHub MUST be closed before
  uploading (it holds the COM port open, upload fails with "Erişim engellendi").

## Reference project — check this FIRST before guessing calibration

An open-source multi-cluster project (linked in the README credits) lists "BMW 3 Series (F30)
6WA" as supported. Its BMW F-series source (`BMWFSeriesCluster.cpp`) was used as a reference
for CAN IDs and layouts; it is a different cluster series, so every value was re-checked here.

**Before reverse-engineering any CAN message by trial and error, check that file for
the same CAN ID first.** This already solved two problems outright:
- Fuel gauge: this repo had `outFuelRange = {22, 7, 3}` (Mini Cooper tank scale).
  The reference project's non-Mini/F-series value is `{37, 18, 4}` (0%, 50%, 100% → raw liters-ish
  value, decreasing = more raw value means less fuel). Using it fixed the gauge on
  the first try after many failed manual attempts (see "dead ends" below).
- Gear P/R/N/D: this repo's `selectedGear` byte (CAN ID `0x3FD`) was only ever set
  for "R" (`0x40`); D was never set, so automatic drive always showed nothing. The reference project's
  encoding: `0x20=P, 0x40=R, 0x60=N, 0x80=D, 0x81=manual/DS`. Fixed by adding those.

Not everything is in the reference project though — it does NOT have: ambient/outside
temperature, a date/time or MID-page-selector message, or a cruise-control
target-speed byte (its `0x289` cruise message is also on/off-only, same limitation
as this repo). Don't expect to find those there; they're genuinely undocumented.

## Confirmed working (tested live on hardware)

- Turn signals, headlights (low/high beam) — SimHub sends booleans as the literal
  text `"True"`/`"False"`, and Arduino's `String::toInt()` silently returns 0 for
  any non-numeric string. Every boolean-sourced field in `simhub/custom_protocol.txt` must be
  wrapped as `if([Prop]==true,1,0)` (NOT `isnull([Prop],'0')`) or it will always
  read as 0 regardless of actual state.
  - **SimHub's formula language uses `if(cond, a, b)`, NOT `iif(...)`.** Using `iif`
    throws a parse error in SimHub's editor. This looks like a VB/.NET-ism that
    doesn't apply here — always use `if(`.
- Headlight logic in `SHCustomProtocol.h`: `light==1 && highbeam==1` (low beam
  still "on" while high beam engaged) was an unhandled combination that fell through
  to "everything off". Fixed by checking `highbeam` first, independent of `light`.
- Fuel gauge: see the reference project values above (`outFuelRange = {37, 18, 4}`). Also learned
  the physical needle only does a clean animated sweep to the new value around a
  **real power-on event of the cluster's own 12V** — cycling the Arduino (e.g. via
  reflashing) does NOT power-cycle the cluster and does not reliably re-trigger the
  sweep. If a gauge looks "stuck", try cutting/restoring the cluster's own 12V
  before concluding the calibration is wrong.
- `simhub/custom_protocol.txt` field 4's fuel-percent formula
  (`DataCorePlugin.GameData.FuelPercent` with a `DataCorePlugin.Computed.Fuel_Percent`
  fallback) **is correctly wired for ETS2** — confirmed live: at
  `DataCorePlugin.GameData.Fuel` = 282.08 / 400L tank, `FuelPercent` read `70.52`
  (i.e. it's already 0-100 scale, matching 282.08/400), so the formula's
  `if(value<=1,*100,*1)` branch takes the correct `*1` path. Don't re-suspect this
  property the way `WaterTemperature` turned out to be wrong for field 3 — it isn't.
  Separately, the physical needle takes a very long time (observed ~8-10 minutes) to
  visibly sweep to a big change (e.g. after refueling to full) even though `0x349` is
  re-sent with the live target value on every `Loop()` cycle. This looks like the
  cluster's own fuel-gauge motor drive is heavily damped/smoothed internally (real
  BMW clusters intentionally do this so the needle doesn't jitter from fuel sloshing
  in the tank) — there is no known CAN message (in this project or the reference project)
  that controls sweep speed, so this is probably not fixable from the Arduino side.
  If a "yakıt bitti ama ibre hâlâ eski konumda" report comes up, first check whether
  it's just this slow sweep still catching up before assuming a calibration bug.
  User confirmed the needle visibly moves much faster specifically during the
  cluster's own 12V power-on/power-off transitions (consistent with the self-test
  sweep behavior noted above) than during normal live driving updates — decided
  (2026-08-20) not to chase a software fix for this (no known CAN "sweep speed"
  control exists, and hacking it via deliberate out-of-range overshoot values was
  considered but rejected as too risky for a purely cosmetic speed gain (2026-08-28
  update: actually tried this despite the earlier rejection - sent a fixed
  out-of-calibrated-range raw value, `0`, against `outFuelRange = {37,18,4}`
  (calibrated 4-37), with ignition forced on. Result: **the needle didn't move at
  all**, not fast, not slow - the cluster appears to just silently reject/ignore
  implausible out-of-range values rather than reacting to them differently. No
  damage or stuck state resulted (confirming the earlier risk assessment was overly
  cautious - out-of-range telemetry values are safe to test, the cluster just
  discards them), but this specific technique is a dead end for controlling sweep
  speed. Don't retry the "overshoot" idea; if this is revisited again, it would need
  real CAN sniffing on a real running car, not more guessing from the Arduino side.
  Given the gauge already settles on the correct value on its own, this stays
  cosmetic-only. Leave as-is.
- **Correction to the "only real 12V power-on triggers a fast sweep" finding above
  (2026-09-02).** That 2026-08-20 finding predates the 2026-08-29 ignition-status
  fix (`0x12F`'s `ignitionStatus` byte was hardcoded `0x8A` always-on before that
  fix - see the "RPM tach message" entry under Confirmed working - so the cluster
  never actually saw a real CAN ignition-off->on transition prior to 2026-08-29,
  only real 12V power-on/off ever produced one). User confirmed the cluster's 12V
  had **not** been power-cycled in 2 days, yet turning the software/SimHub ignition
  on today still produced a fast, clean sweep straight to the real (accurate, not
  stuck-at-sweep-max) fuel level once the 2-second welcome-sweep window ended -
  which the old finding can't explain, but a **CAN-level ignition on-transition**
  (now sent correctly since the 0x8A/0x8 fix) plausibly can: real BMW clusters
  normally do their needle self-test sweep on key-on, not only after a battery
  disconnect. Refueling *while already driving* (ignition steady, no 0->1
  transition) never gets this fast treatment, matching everything observed so far.
  **Not yet tested, but worth trying before assuming this is still unfixable:**
  briefly pulsing `EngineIgnitionOn` off then back on (a fake key-cycle, faked
  entirely in `SHCustomProtocol::Loop()`/`read()`, no real hardware power-cycle
  needed) right after detecting a large fuel increase might force the cluster into
  its fast-sweep mode for the fuel needle specifically. Caveat before wiring this
  up: it would also re-trigger the *whole* welcome sweep (speed/RPM/temp needles
  jumping to max too) and possibly other real-car ignition-cycle side effects
  (warning lamp self-tests, trip-related resets) - discuss the tradeoff with the
  user before implementing, don't just wire it silently.
- **Odometer (total km) inflating way too fast — fixed (2026-08-20).** User reported
  the cluster's real odometer jumped from 438,xxx to 439,xxx km (~1000km) after only
  a few minutes of actual driving. Root cause: `distanceTravelledCounter +=
  Speed*2.9` (CAN ID `0x2BB`) in `SHCustomProtocol::Loop()` ran completely
  unthrottled — on *every single call* to `Loop()`, which itself runs as fast as the
  Arduino's main `loop()` can cycle (bounded only by SPI/CAN send time for the ~20
  other messages sent per iteration), likely tens to hundreds of times per second.
  Checked the reference project: its otherwise-identical `distanceTravelledCounter
  += speed*2.9` line only runs inside `sendDistanceTravelled()`, itself gated behind
  a real 100ms (`dashboardUpdateTime100`) `millis()` timer — i.e. a fixed 10Hz rate,
  not once per raw loop iteration. Fixed by wrapping the whole `0x2BB` send +
  increment block in `SHCustomProtocol::Loop()` with the same
  `if (millis() - lastDistanceUpdateMs >= 100) { ... }` gate. **If any other
  odometer/mileage-related weirdness shows up, check first whether the responsible
  CAN send is properly time-gated like this — most of this sketch's other messages
  are level/state signals where sending them faster than needed is harmless, but
  this one is a running accumulator where send rate directly IS the physical rate of
  change, so it's uniquely sensitive to this class of bug.**
- RPM under-reading (e.g. real 1000rpm showing as 750): caused by `GRPMOND = true`
  in `bmw_f30_cluster.ino`, which applies a nonlinear compensation curve meant for "a
  diesel cluster with a mismatched petrol RPM disc" — a specific hardware situation
  from the original author, not applicable here. Set `GRPMOND = false` for a
  straight 1:1 RPM mapping.
- Trip/menu button on the cluster itself (not wired to any Arduino GPIO) reports its
  state over CAN, not via `ENABLED_BUTTONS_COUNT`/GPIO like the sketch's built-in
  "Additional Buttons" feature. Found by sniffing: **CAN ID `0x5E0`**, idle/heartbeat
  payload `11 03 00 01/02 FF FF 00 FF` (byte[3] just an unrelated alive counter),
  and byte[0] becomes `0x8C` while the button is actively pressed. Wired into
  `bmw_f30_cluster.ino`'s `loop()` as a permanent `CAN.readMsgBuf` listener that calls
  `buttonStatusChanged(1, ...)` when byte[0] toggles between `0x11`/`0x8C`; also
  bumped `ENABLED_BUTTONS_COUNT` to `1` so SimHub's Controls & Events screen sees it
  as "Button 1". The sketch never reads CAN otherwise (only sends) — this was the
  first and only receive path added.
- Seatbelt lamp: `0x5c0` message, warning code `71` and `77`, byte[3] `0x28`=off /
  `0x29`=on (this on/off convention is used by every warning on `0x5c0`, e.g. check
  engine uses code `34`, DSC uses code `215`).
  **Caution:** forcing the seatbelt lamp on (`0x29`) was empirically observed to also
  turn the parking-brake lamp on by itself; reverting it fixed the parking-brake
  lamp too. Not fully explained (possibly code `71`/`77` isn't really "seatbelt" in
  BMW's real fault-code table, or the two lamps share cluster-internal state) — if
  the user wants the seatbelt lamp forced on again, warn them the handbrake lamp
  may light up as a side effect and needs to be re-verified.
- **Remote button to change the MID/trip screen — confirmed working (2026-08-20).**
  Unlike the read-only `0x5E0` trip button above, the cluster also *listens* for
  CAN ID `0x1EE` — the reference project's `BMWFSeriesCluster::sendSteeringWheelButton()`
  message, i.e. the BMW F-series steering wheel MENU button. Payload `{76, 0xFF}`
  while held / `{0, 0xFF}` on release makes the cluster advance its own MID/trip
  page, same as the real steering-wheel button would. Implemented as a second GPIO
  button in `bmw_f30_cluster.ino`: `ENABLED_BUTTONS_COUNT` bumped to `2`, `BUTTON_PIN_2` =
  Arduino pin 4 (momentary pushbutton between D4 and Arduino GND, INPUT_PULLUP, no
  external resistor needed), CAN send added inside `buttonStatusChanged()` for
  `buttonId == 2`. The reference project's source has no enum for `buttonEvent` — only `==1` means
  pressed — so this is a single generic "next page" cycle button; there is no
  known/documented "previous page" CAN message anywhere (not in this project, not
  in the reference project) — would need fresh CAN sniffing of a real steering wheel to
  find one, if the cluster even supports going backward at all.
  **Build gotcha that caused the first attempt to silently do nothing:** the whole
  `ENABLED_BUTTONS_COUNT`/`BUTTON_PIN_*` "Additional Buttons" feature block in
  `bmw_f30_cluster.ino` is wrapped in `#ifdef INCLUDE_BUTTONS`, and `INCLUDE_BUTTONS` is
  normally injected by **SimHub's own Arduino sketch generator** (based on which
  peripheral checkboxes are ticked in its config UI) — it is never `#define`d
  anywhere in this repo's source files. Since this project is compiled directly
  with `arduino-cli` (see Hardware/setup above), that generator step never runs, so
  every `INCLUDE_*`-gated optional peripheral (buttons, encoders, TM1638/TM1637,
  LCD, RGB strips, etc.) silently compiles out to nothing — the board flashes fine,
  no error, the pin is never even set to `INPUT_PULLUP`, and nothing happens when
  you press the button. Fixed here by adding a plain `#define INCLUDE_BUTTONS` right
  above its first `#ifdef` use. **If any other `INCLUDE_*`-gated peripheral feature
  is ever wired up on this board (rotary encoders, etc.), it will need the same
  manual `#define INCLUDE_<FEATURE>` treatment** — check first whether the feature's
  `#ifdef` guard is actually defined anywhere before assuming new code is broken.

- **Language byte — Turkish found (2026-08-28).** `setlanguage` in `bmw_f30_cluster.ino` (sent as
  byte[0] of CAN ID `0x291` in `SHCustomProtocol.h`) was only documented for `1=German`,
  `2=English` by both this repo's original author and the reference project. Neither
  source lists any other values. Tested live on the real Türkiye-spec cluster by flashing
  `setlanguage = 0` and power-cycling the cluster's own 12V (needed for the setting to take):
  **`0 = Turkish`**. Confirmed working. If further language codes are ever needed, the same
  method (edit `setlanguage`, reflash, power-cycle cluster 12V, observe) is the only known way
  to find them — this byte's mapping isn't documented anywhere else that we've found.

- **Drive mode (Traction/Comfort/Sport/Sport+/DSC off/Eco Pro) live cycle button
  (2026-08-28).** `drivemod` (`bmw_f30_cluster.ino`) is sent on CAN ID `0x3A7` and was a
  compile-time-only constant, same pattern as `setlanguage`. User wanted to change it
  live while driving, ideally via a SimHub keyboard binding. Investigated SimHub's
  "Controls and events" for a way to bind a key to a persistent custom variable
  readable from a formula: enabled "Enable device authoring tools" in Settings, but
  neither `custom` nor `variable` appeared anywhere in the Target search, and
  SimHub's own docs/wiki confirm its NCalc formula engine has no cross-formula
  variable storage — this isn't cleanly supported in stock SimHub. Went with a
  physical-button approach instead (user's choice over trying a 3rd-party
  "Computed Properties" plugin): added **Button 3**, GPIO pin **D5** (GND +
  `INPUT_PULLUP`, no resistor), `ENABLED_BUTTONS_COUNT` bumped to 3. Each press
  (on press only, not release — see `buttonStatusChanged(buttonId==3, ...)`) cycles
  `drivemod` through `{1,2,4,5,6,7}` (wraps). Entirely local to the Arduino, no
  SimHub config needed. Also separately wired `simhub/custom_protocol.txt` field 53 (previously
  unused `dmode`, parsed but dead) to override `drivemod` live if SimHub ever sends
  a nonzero value 1/2/4/5/6/7 (3 snaps to 4) — this is for a **future, different**
  game that exposes a real `TCLevel`/engine-map-style property (ETS2 doesn't have
  one, so this path is currently inert for this project's actual use case; revisit
  the field 53 formula together once a game with that kind of data is actually
  used).
- **Drive mode cycle from a wheel button via SimHub's raw `JoystickPlugin.*`
  properties (2026-09-01, needs live confirmation).** User wanted the drive-mode
  cycle triggerable from a button on their sim racing wheel (Logitech G29) instead
  of reaching over to the dash-mounted D5 button (also has an STM32 button box, but
  decided to use the wheel button rather than route through that). The 2026-08-28
  entry above already found stock SimHub's "Controls and events" has no generic
  "bind input to an arbitrary custom property" target action (re-confirmed live:
  searching the Target picker for `custom`, `property`, and `serial` all came up
  empty; `GraphicalDashPlugin`'s `ActionA-D` looked promising but aren't exposed as
  readable properties either - confirmed via Settings > Available properties,
  searching `Action` returned nothing). **Working route found:** every
  joystick/wheel/button-box button's raw press state is directly readable as a
  property under `Available properties > InputStatus`, named
  `JoystickPlugin.<device name with underscores>_<button id>` - confirmed live for
  the user's G29, `JoystickPlugin.Logitech_G_HUB_G29_Driving_Force_Racing_Wheel_USB_B20`
  goes 0->1 when the button is held. **First attempt without a null guard failed**
  live in SimHub's Custom Serial Device "Protocol message binding" editor with
  `Expression error: Value cannot be null. Parameter name: conversionType` (the
  unmodified original formula did not error, only the version with the bare
  `[JoystickPlugin...]>0` comparison did) - a GPIO-button fallback (Button 6, pin
  D8) was tried and reverted again once the likely real cause was spotted: every
  other numeric field in this formula wraps its property in `isnull([X], default)`
  before doing math/comparison on it (see e.g. field 2's
  `isnull([DataCorePlugin.GameData.NewData.Rpms],'0')`) and the joystick line
  didn't - NCalc likely can't implicitly compare a null to a number. Retried as
  `if(isnull([JoystickPlugin.Logitech_G_HUB_G29_Driving_Force_Racing_Wheel_USB_B20],0)>0,1,0)`
  in `simhub/custom_protocol.txt` field **52** (`dmode` - this is field 52, not 53; corrects an
  off-by-one in the 2026-08-28 entry and `bmw_f30_cluster.ino`'s old comment, recounted
  against `SHCustomProtocol.h::read()`'s actual parse order). `read()` tracks this
  with a `prevDmode` static and cycles `drivemod` through `driveModes[]` (same
  list/logic as the GPIO Button 3 handler, `extern`-shared via
  `driveModes[]`/`driveModesCount`) on the 0->1 edge only, so holding the wheel
  button doesn't advance repeatedly. GPIO Button 3 (D5) is untouched and still
  works too - both drive the shared `drivemod` variable without conflicting.
  **Not yet confirmed whether the `isnull()` fix actually clears the
  conversionType error** - next session, check this first before assuming it
  works. **Property name is wheel/button-specific** - if the user rewires this to
  a different physical button, a different wheel, or the STM32 button box instead,
  the exact `JoystickPlugin.*` property name changes; find the new one live via
  Available properties (search a keyword matching the device, press the button,
  watch which row's value changes) rather than guessing.
- **Button 4, GPIO pin D6 (2026-08-28):** consumption-unit cycle button, same pattern
  as button 3. Press cycles `setmpgl100orkml` through `1=l/100km, 2=mpg, 3=km/l`
  (wraps); `setkmormiles` untouched. See `buttonStatusChanged(buttonId==4, ...)`.
  Buttons in use so far: 1=cluster's own trip button (CAN, not GPIO), 2=D4 (MID/menu
  page), 3=D5 (drive mode cycle), 4=D6 (consumption unit cycle), 5=D7 (real seatbelt
  buckle switch, see below). Next new physical input should use D8 or higher
  (D2/D10/D11/D12/D13 are taken by the MCP2515 CAN shield's INT/CS/SPI lines - never
  reuse those).
- **Real seatbelt buckle switch wired in (2026-08-28), not yet tested live.** Button
  5, GPIO pin D7 (GND + `INPUT_PULLUP`), reads the actual seatbelt buckle switch as a
  continuous level (not a press event - see `buttonStatusChanged(buttonId==5, ...)`,
  sets global `seatbeltBuckled`). Drives both `0x5c0` warning codes 71 and 77 (both
  were previously hardcoded permanently "off"): `seatbeltBuckled ? 0x28 : 0x29`.
  Assumes the switch shorts to GND when the belt IS buckled - if this specific
  buckle's switch is wired the opposite way, flip the ternary in both places (search
  `seatbeltBuckled ?` in `SHCustomProtocol.h`). Defaults to `seatbeltBuckled = 1`
  (buckled/lamp off) until the physical switch is actually connected, so this is
  backward-compatible with the prior always-off behavior until D7 is wired.
  **RESOLVED (2026-08-28) by isolating which of the two `0x5c0` codes causes the
  parking-brake coupling.** Tested each alone with the real switch: **code 71 on
  its own is enough to also toggle the parking-brake lamp** (confirmed reproducible
  - this is a real coupling inside the KOMBI's own fault-code table, not a CRC/byte
  bug in our code); **code 77 on its own is clean** - toggles only the seatbelt
  lamp, parking-brake lamp unaffected. Final state: **code 71 is permanently
  hardcoded off** (`0x28`, never touches the switch) and **only code 77 is driven
  live by `seatbeltBuckled`**. Separately confirmed the real parking-brake lamp
  (CAN `0x34F`, driven by the `handbrake` variable from
  `DashboardValues.ParkingBrake`) still works correctly on its own once code 71 was
  taken out of the picture (tested by temporarily forcing `handbrake = 1` in
  `Loop()` and confirming the lamp lit, then reverted). Feature is done and safe to
  use as shipped - do not re-enable code 71 from the switch.
  **Mystery explained (2026-08-29):** found the reference project's own comments for
  `0x5c0` fault codes - code **71 is actually "Park brake error (red)"**, not
  seatbelt at all; this project's original author simply mislabeled it
  (`seat_belt_indecator` variable name, comment "seat belt indecator") — that's why
  it moved the parking-brake lamp, it never was seatbelt-related. Code **77 really
  is "Seat belt indicator"**, confirming why it tested clean. Full known code list
  from the reference project, useful for any future `0x5c0` work: **24** = park brake
  error (yellow), **34** = check engine, **35** and **215** = DSC, **36** = DSC off,
  **71** = park brake error (red), **77** = seat belt indicator. These are the
  *only* `0x5c0` codes documented anywhere (this project or the reference project's) - any other
  warning lamp not in this list is unmapped territory, would need real trial/error
  or CAN sniffing to find (see the check-engine icon note in Known broken below).
- **`arduino-cli upload` failing with `cannot set com-state for \\.\COM8` /
  `unable to open port COM8` even though SimHub and every other known app is
  closed (2026-08-28):** `mode COM8` (plain `cmd /c "mode COM8"`) still succeeded
  and reported the port config even while upload was failing, so this wasn't
  actually another process holding the port — it's a known CH340-on-Windows glitch.
  Fix: physically unplug and replug the Nano's USB cable, then retry the upload
  immediately. Worked first try. If this recurs, try that before hunting for a
  phantom process holding the port.

- **Tachometer OFF/READY dial state — fixed (2026-08-28), needs live confirmation.**
  This cluster's tach face is printed "OFF READY 0 1 2 3..." instead of plain
  numbers. User wanted OFF when ignition off, READY when running. Root cause: the
  `0x12F` ignition/terminal-status message's `ignitionStatus` byte was hardcoded to
  `0x8A` (the "on" value) always, regardless of real game state - so the dial could
  never show OFF. Checked the reference project's `sendIgnitionStatus()`:
  `ignitionStatus = ignition ? 0x8A : 0x8`. Fixed by tying it to the existing
  `EngineIgnitionOn` variable (already parsed from SimHub, `simhub/custom_protocol.txt` fields
  27/33): `(EngineIgnitionOn == 1) ? 0x8A : 0x8`. **OFF state confirmed live**
  (2026-08-28, SimHub closed so `EngineIgnitionOn` defaults to 0 -> dial showed
  OFF, as expected). READY/running transition not yet tested (needs SimHub+ETS2
  running with ignition on) - confirm that next time the cluster is used live.
  Caution found while researching this: `EngineIgnitionOn` gets force-set to `1`
  at `SHCustomProtocol.h` (`if(checkengwhenoff == false) { EngineIgnitionOn = 1; }`)
  - harmless with the default `checkengwhenoff = true`, but if that flag is ever
  turned off this ignition fix would stop working (tach would get stuck "on")
  since it'd permanently overwrite the variable this fix now also depends on.

- **Backlight now tied to ignition instead of headlights (2026-08-28), confirmed
  live.** Panel backlight brightness (CAN `0x202`) was previously only sent while
  `light==1` (headlights on) - dark whenever headlights were off even with ignition
  on. User wanted it always on whenever ignition is on, regardless of headlights.
  Moved the `0x202` send out of the `light`/`highbeam`/`fogg` if-else chain (which
  still only controls the `0x21A` light-state message) into its own
  `if (EngineIgnitionOn == 1) { ... }` block right after. Confirmed live: backlight
  now comes on immediately with ignition, independent of headlight state. Note:
  during testing, backlight also had a delayed self-recovery after a reflash even
  on the OLD code (came back "on its own after a while") - consistent with the
  fuel-needle sweep behavior already documented above (some cluster state seems to
  settle/refresh on a delay independent of what's actively being sent); don't
  assume something is broken just because a change doesn't show instantly right
  after a fresh flash.
- **Parking-brake lamp can get stuck showing a stale state after manual CAN testing
  - needs the cluster's own 12V power-cycled to clear, not just the Arduino
  (2026-08-28).** While testing the seatbelt fix above, temporarily forcing
  `handbrake = 1` then reverting left the parking-brake lamp lit even with ignition
  off, even though the code provably sends an explicit "off" frame on every fresh
  boot (`Setup()` sets `prevhand = 1` while `handbrake` defaults to `0`, so the
  mismatch forces one real send). Power-cycling the cluster's own 12V (not the
  Arduino) cleared it. Same class of issue as the already-documented fuel-gauge
  needle only doing a clean sweep on a real cluster power-on - if any lamp/gauge
  looks "stuck" after manual testing with forced values, try a real 12V cycle on
  the cluster before assuming the CAN message logic is wrong.
  **Recurred again (2026-08-28) after further reflashes with no forced value at all
  in the code** - confirms this isn't specific to the deliberate handbrake test,
  it's a general "Arduino reflash != cluster reset" issue. **Practical rule going
  forward: after any session involving several back-to-back reflashes while testing
  warning lamps, do one real 12V power-cycle on the cluster at the end before
  trusting what the lamps show** - don't chase phantom lamp bugs in code without
  ruling this out first.

- **RPM tach message: tried the reference project's single-byte structure, reverted back
  (2026-08-28 → reverted 2026-08-29).** User reported the tach needle moves
  realistically with RPM but never goes above ~2000rpm even though some ETS2 trucks
  reach ~3000rpm. Found the `0xF3` message's byte layout was structurally different
  from the reference project: our code sends RPM as a raw 16-bit value split
  across bytes[1-2] (`int(RPM2*1.557)`, `& 0xff` / `>> 8`) using a separate custom
  `crc8()` function and a dual-send-per-`Loop()` pattern (same value sent twice, at
  `RPM2+4` then `RPM2`); the reference project's real `sendRPM()` instead sends a **single byte**
  mapped `map(rpm, 0, 6900, 0, 0x2B)` (0-43) at a different byte position, plus a
  gear byte, using `crc8Calculator.get_crc8(..., 0x7A)`. Tried rewriting to match
  the reference project's structure (`rpmMapped = map(RPM2, 0, 7500, 0, 0x2B)`, kept this project's
  own `efficient` eco-light byte, gear byte placeholder `0x00`, added an EMA
  smoothing filter to compensate for the coarser 44-step resolution) - **confirmed
  live this did fix the >2000rpm cap**, but the needle no longer felt right overall
  (user: "tam olmadı", wanted the old feel back) even with the smoothing filter.
  **Reverted (2026-08-29) to the original 16-bit encoding + dual-send pattern**
  rather than keep patching blind - current code in `SHCustomProtocol.h` is back to
  exactly what it was before 2026-08-28's RPM changes. **If this is revisited:**
  don't just reapply the same single-byte rewrite as before - it fixed the range
  but the feel tradeoff wasn't accepted. Would need either a smarter middle ground
  (finer resolution than 0x2B if the real field width is actually wider than the reference project's
  own calibration assumes) or real hardware confirmation of what this specific
  cluster's `0xF3` field actually expects, rather than assuming the reference project's exact
  numbers transfer over. The >2000rpm cap itself is still unsolved as of this
  revert.
  Related, not fixed: `idleRPM += 10;` runs unconditionally every `Loop()` call with
  no reset and no bound - given `Loop()` runs unthrottled (see the odometer note
  above), this will overflow `int` range within roughly a minute of runtime. Only
  feeds the `efficient` eco-light comparison (`RPM > idleRPM`), not the RPM value
  itself, so it doesn't explain the 2000rpm cap, but is still a real latent bug
  worth fixing separately if the eco light ever seems to behave strangely after the
  cluster's been running a while.

- **"Welcome sweep" on ignition-on — confirmed working (2026-08-29).** User wanted
  the needles to sweep to max and back whenever ignition turns on, like a real
  self-test. Added at the top of `SHCustomProtocol::Loop()`: detects
  `EngineIgnitionOn` going 0->1, opens a 2-second `inSweepWindow`, during which
  speed/RPM/temp/fuel gauge sends are overridden to max instead of their real
  values. Speed/RPM use the dial's actual **printed** max (260km/h, 6000rpm -
  confirmed by the user reading the physical dial), not the higher software
  ceilings (350/7500) used during normal driving - those ceilings are fine for
  clamping live data but would push the needle past its visual max if used as a
  literal sweep target. Fuel is included too (`fuelQuantityLiters = 4`, the "full"
  raw code) but may not visibly keep up given the fuel gauge's already-documented
  heavy damping. Confirmed working live by the user.
- **Live confirmation (2026-09-22): only speed and RPM visibly sweep, fuel does
  not.** User reports that on the real cluster, the welcome sweep is only
  visually noticeable on the speed and RPM needles - the fuel needle does not
  appear to participate. This matches the prediction already written above
  (fuel's target is included in the sweep send, but the needle's own heavy
  hardware damping means it can't visibly react within the 2-second window) -
  not a bug, expected given everything already documented in the fuel-sweep
  saga elsewhere in this file. Whether the repurposed temp/pressure needle
  (0x3f9) visibly sweeps was not confirmed either way this session - unlike
  fuel, that gauge has no known heavy hardware damping (its EMA smoothing is
  software-side and is bypassed during the sweep window via `tempForDisplay =
  inSweepWindow ? 200 : Temp`), so there's no a priori reason to expect it to
  behave like fuel - check this specifically next time the welcome sweep is
  observed live, don't assume it's silently excluded too just because fuel is.
  **Confirmed same session: the temp/pressure needle (0x3f9) does visibly move
  during the sweep.** So the welcome sweep is visibly working on three of the
  four gauges it targets (speed, RPM, temp/pressure) - fuel is the sole
  exception, and specifically because of its already-documented unique
  hardware damping, not because of anything wrong with the sweep code itself.

- **Fuel "force empty then real" ignition-on test — implemented and iterated live
  (2026-09-22); works, but only with several manual ignition cycles, not one.**
  Following the "fast-sweep-if-starting-from-true-empty" data point earlier the
  same session, user asked to deliberately test whether telling the cluster the
  tank is EMPTY for a few seconds right after ignition-on, then switching to the
  real target, makes the needle's subsequent climb faster/more complete than the
  already-documented slow crawl. Implemented in `SHCustomProtocol.h`'s `Loop()`: a
  separate timed window (independent of the existing speed/RPM/temp welcome-sweep
  window) that forces `fuelQuantityLiters = 37` (the calibrated empty value from
  `outFuelRange`, not an out-of-range hack) for a fixed duration after the
  `EngineIgnitionOn` 0->1 edge, then lets the real `multiMap()`-computed target
  take over. This **replaces** the old "force full (4) during the general welcome
  sweep" fuel behavior (already confirmed above to not visibly move the needle at
  all - no point keeping both).
  **v1 (2.5s window):** user manually cycled ignition several times within ~10s
  total and reported the tank reached full. Initial theory: the window-start logic
  resets on every single ignition 0->1 edge, so rapid repeated cycling kept
  re-arming the 2.5s timer each time, keeping `fuelQuantityLiters` pinned at 37
  for close to the whole ~10s - i.e. total *held-at-empty duration* was the
  variable that mattered, not the number of distinct cycles.
  **v2 (widened the window to 9s to test that theory in one cycle) — theory
  disproven.** A single real ignition on/off with the 9s window only reached
  **~75%**, no better than other single-cycle attempts elsewhere in this file -
  a continuous 9s hold did not reproduce what several shorter, repeated holds did.
  **Reverted back to v1 (2.5s window)** per the user's explicit request - it's the
  last configuration with a confirmed full-fill result, even though it still
  requires several manual ignition cycles in quick succession to get there.
  **Open question, unsolved:** the real mechanism isn't duration-at-empty, it's
  something specifically about *multiple distinct ignition-on transitions* in a
  short window - possibly each 0x12F `ignitionStatus` 0x8->0x8A edge re-triggers
  some cluster-internal fast-sweep-eligibility state that decays, and needs
  re-arming more than once before the fuel gauge specifically responds to it
  (speed/RPM/temp only ever needed one edge, per the original welcome-sweep
  entries above - this may be fuel-specific). **Don't re-try "just make the
  single-cycle window longer" without new evidence - that was tested and
  failed.** If revisited, a more promising angle might be deliberately sending 2-3
  distinct fast on/off/on pulses on the `0x12F` ignition message itself within one
  real key-on event (faking the multi-edge pattern that worked, rather than
  faking a longer single hold) - not implemented, discuss with the user first
  since messing with `0x12F` touches the tach OFF/READY state too, not just fuel.
  **v3 (same session, right after v2/v1): widened to 20s, on a different theory
  than v2's.** User's own hypothesis: v2's 9s hold may simply not have been long
  enough for the needle to physically reach the true empty raw value (37) at all,
  given how slow this gauge is documented to be everywhere else in this file (full
  swings normally take ~8-10 minutes) - so the "return swing" in the 9s test may
  have started from a position still fairly close to full, not genuinely empty,
  which would explain why it capped at the same ~75% as everything else. This is
  a different claim than v2's (which was about total duration mattering
  regardless of whether true empty was reached) and hadn't been tested yet, so
  widening again here doesn't contradict the "don't just widen the window without
  new evidence" caution above - this has new reasoning behind it. Compiled and
  uploaded to the board (COM19) - **not yet confirmed live.** If 20s ALSO caps
  around ~75%, that's fairly strong evidence the ~75% figure is an intrinsic
  property of the needle motor's own deceleration curve (matches the un-hacked
  2026-09-21 natural-refuel observation, which also capped near 75%, with no
  software trick involved at all) rather than something fixable by holding at
  empty longer - at that point this whole "hold at empty" angle should probably
  be considered closed, since holding meaningfully longer than 20s starts
  reintroducing the multi-minute wait this approach was trying to avoid in the
  first place. **v3 result (confirmed live, 2026-09-22): did NOT reach full**
  ("olmadı" - no exact number given, but not fixed). This is meaningful evidence:
  neither widening direction tested so far (v2's 9s, v3's 20s) beat v1's original
  2.5s-with-multi-cycling result, which weakens the whole "duration at empty" idea
  regardless of which specific sub-theory (v2's vs v3's) was being tested.
  **v4 (this session, same day): swung the other direction - narrowed to ~1.5s.**
  Since neither longer duration helped, testing whether a brief flicker to empty
  (short enough the needle barely has time to move at all, more a quick "flag"
  than a real excursion) does something different from a sustained hold, rather
  than continuing to guess further along the "longer is better" axis. Compiled and
  uploaded to the board (COM19). **v4 result (confirmed live, 2026-09-22): only
  ~25%, worse than v2's 9s (~75%).** This is the key data point that reframed the
  whole approach: percentage-reached tracked roughly with hold duration (1.5s→25%,
  9s→75%, 20s→still not full) rather than being flat/capped independent of
  duration - so duration clearly does affect how far a single cycle gets, but
  no duration tested ever reached the full 100% that the original multi-cycle
  test hit. Conclusion: a single continuous hold, however long, reliably
  under-performs several distinct real ignition cycles - confirming the
  "multiple distinct ignition-on edges" theory over "total time held at empty"
  and closing off the duration-tuning line of testing (1.5s/2.5s/9s/20s all
  tried, don't re-try more values on this axis without new evidence).
  **v5 (this session): stopped tuning duration, synthesized the multi-edge
  pattern directly.** Implemented in `SHCustomProtocol.h`: within ~2s of a real
  ignition-on, the `0x12F ignitionStatus` byte is pulsed through 2 extra fake
  off→on transitions (5×400ms segments: ON,OFF,ON,OFF,ON) via a new
  `ignPulseForceOff` flag, layered on top of (not replacing) the real
  `EngineIgnitionOn`-driven value - only the CAN byte is faked, the actual
  `EngineIgnitionOn` variable is untouched, so nothing else keyed off real
  ignition state (backlight, welcome sweep trigger, etc.) is affected.
  `fuelQuantityLiters` is pinned at empty (37) for that same ~2s window
  (`inIgnPulseWindow`, replaces the old duration-only `inFuelEmptyTestWindow`).
  **Known, accepted side effect:** the tach's OFF/READY display is driven by this
  same `ignitionStatus` byte, so it will visibly flicker OFF/READY/OFF/READY
  during this ~2s window - user was told this before agreeing to test it.
  Compiled and uploaded to the board (COM19). **v5 result (confirmed live,
  2026-09-22): only ~25%, identical to v4's plain-duration-only result at a
  similar duration (~1.5-2s).** The fake ignition-on edges added nothing
  measurable beyond what the hold duration alone already predicted - this
  disproves the "multiple distinct ignition-on edges" theory too, not just the
  duration theories from v2-v4.
  **SESSION CONCLUSION (2026-09-22), settled on v1 as the final version:** across
  v1-v5, single-cycle attempts topped out around ~75% (v2's 9s) and some went as
  low as ~25% (v4, v5) - none reliably reached 100% in one real ignition cycle,
  regardless of hold duration (1.5s/2.5s/9s/20s) or whether fake extra ignition
  edges were layered on top. Only the very first v1 observation (several real
  manual ignition cycles within ~10s, with the 2.5s window in place) ever reached
  full, and every later attempt to isolate or reproduce *why* that worked (longer
  holds, shorter holds, synthesized edges) failed to reproduce it again - the
  most defensible read at this point is that the original "full" result was more
  likely coincidental elapsed real time (consistent with this gauge's
  already-documented ~8-10 minute natural settle time) than something the empty-
  hold hack itself caused. **Reverted to v1 (2.5s window, no ignition-pulse
  hack)** per explicit user request - it's the simplest version that at least
  matches every other variant's ceiling, and is the one version with a (likely
  coincidental, but real) full-fill anecdote behind it, needing 1-2 manual
  ignition cycles to get there. **Don't restart this duration-tuning or
  edge-faking rabbit hole without a genuinely new idea** - the parameter space
  (duration: short/medium/long; edges: none/faked) has been reasonably well
  covered this session. If a single-cycle full fill is wanted again in the
  future, the most promising unexplored angle is probably replicating a REAL
  off period (not just toggling the `ignitionStatus` CAN byte while
  `EngineIgnitionOn` stays 1) - e.g. actually zeroing out speed/RPM/etc. and
  treating it as a genuine momentary ignition-off state - since that's the one
  aspect of a real manual key-cycle none of v1-v5 actually replicated, and real
  power-cycles are separately documented elsewhere in this file as behaving
  differently from any CAN-message trick tried so far. Current shipped state:
  v1, 2.5s window, in `SHCustomProtocol.h` - commented accordingly at both the
  window-timer setup near the top of `Loop()` and the `0x349` fuel send further
  down. Still unconfirmed either way: whether the forced-empty window trips a
  spurious low-fuel warning lamp/chime even when the real tank is full - watch
  for this next time it's tested live.

- **Average consumption / range MID page reset via long-press — confirmed working
  (2026-09-19).** User reported the displayed average-consumption/range number on
  the MID screen looked too high (see the "Range/consumption bar erratic" entry
  below for the unfixed root cause — ETS2 truck fuel/distance ratio doesn't match
  what the cluster, calibrated for a real BMW tank, expects). No CAN message exists
  to set/scale this number directly (still true, see below), but this project
  already wires **Button 2 (D4)** to hold CAN `0x1EE` (`{76,0xFF}`) continuously
  for as long as it's physically held (`menuButtonHeld` in `bmw_f30_cluster.ino`,
  specifically left this way to support "cluster's own long-press-to-reset-trip-data
  behavior", same as a real BMW's steering-wheel MENU button long-press). **Tested
  live: cycling to the average-consumption/range MID page with short presses, then
  holding D4 for a few seconds, resets that trip's average-consumption counter to
  zero and it starts recalculating from that point** — exactly like a real BMW trip
  computer reset. No code change needed, this was already-implemented behavior.
  **Caveat:** this is a reset, not a permanent fix — because ETS2's truck
  fuel/distance ratio still doesn't match a real BMW's, the number will likely climb
  back toward an inflated value again over time/distance after the reset. If the
  user wants a lasting fix instead of a periodic manual reset, that still needs the
  not-yet-found "set average consumption" CAN message (see below) via real CAN
  sniffing on a donor car — no shortcut around that found yet.

- **ROOT CAUSE FOUND (2026-09-19) for why the wheel-button-to-`dmode` binding never
  worked, on the G29 *or* the Moza: `[JoystickPlugin.<device>_<button>]` is the
  wrong property reference inside an NCalc formula. The correct reference needs an
  `InputStatus.` category prefix: `[InputStatus.JoystickPlugin.<device>_<button>]`.**
  This had been silently broken since the very first 2026-09-01 attempt - that
  session's own notes already flagged "not yet confirmed whether the isnull() fix
  actually clears the conversionType error", and it turns out the fix never
  actually worked, the whole G29-era binding was dead the entire time. Found by
  methodically bisecting where the signal was lost: (1) SimHub's **Available
  properties** browser showed the raw button state flipping 0/1 correctly - so
  SimHub sees the hardware fine; (2) a **live preview of the actual Custom Serial
  Device output string** (temporarily moving the field to the front of the formula
  so it's visible without scrolling) stayed at `0` even while holding the button -
  so the break was in the formula, not downstream in serial/Arduino; (3) the
  **Ncalc Tester**'s own **"Insert property"** button (searching `B02` and letting
  SimHub auto-generate the reference, instead of hand-typing it) revealed the
  actual required syntax includes `InputStatus.` - manually typing
  `[JoystickPlugin...]` without it silently evaluates to `null` forever, no error
  thrown anywhere, which is exactly why this was so hard to catch. **Confirmed
  live in the Ncalc Tester: `if(isnull([InputStatus.JoystickPlugin.R3_Racing_Wheel_and_Pedals_B02],0)>0,1,0)`
  returns `1` while held**, vs. always `0` without the prefix. `simhub/custom_protocol.txt`
  field 52 fixed to include the prefix.
  **Standing lesson: any `JoystickPlugin.*` (or likely any other InputStatus-category)
  property referenced in an NCalc formula needs the `InputStatus.` prefix - the
  Available Properties browser does NOT show this prefix in its list, which is the
  trap. When adding a new one, don't hand-type the bracket reference from what's
  shown in Available Properties - use the formula editor's own "Insert property"
  button and copy exactly what it generates, or explicitly prepend `InputStatus.`
  yourself.** GameData/DataCorePlugin telemetry properties (the vast majority of
  this project's fields) are a different category and do NOT take this prefix -
  this is specific to InputStatus-sourced ones (joystick/wheel/button-box raw input
  state).
- **Wheel changed from Logitech G29 to a Moza R3 (with a real BMW F30 steering
  wheel rim) — the 2026-09-01 drive-mode wheel-button entry's property name is
  stale (2026-09-19).** User no longer uses the G29; `simhub/custom_protocol.txt` field 52 was
  still pointing at `JoystickPlugin.Logitech_G_HUB_G29_Driving_Force_Racing_Wheel_USB_B20`,
  a device that no longer exists, which is why "drive mode wheel button" reportedly
  never worked — not a logic bug, just a stale device name after a hardware swap.
  Found live in SimHub (search `Joystick` in Available properties after confirming
  the Moza base shows up in Windows `joy.cpl` first — plain `Moza` search initially
  returned nothing, broader `Joystick` search did) that the Moza's SimHub device
  name is `R3_Racing_Wheel_and_Pedals`. The wheel rim is a genuine BMW F30 wheel, so
  its physical buttons are literally labeled the same as this cluster's own
  functions — the button labeled **"MODE"** is `JoystickPlugin.R3_Racing_Wheel_and_Pedals_B02`,
  now wired into field 52 (replacing the dead G29 property), still driving
  `drivemod` cycling via the existing edge-triggered logic in `SHCustomProtocol.h`'s
  `read()` — no Arduino-side code change needed, only the `simhub/custom_protocol.txt` property
  name. **Needs the user to paste the updated `simhub/custom_protocol.txt` into SimHub and
  confirm the MODE button now cycles drive mode live** (not yet confirmed as of
  this edit). **General lesson: if any `JoystickPlugin.*`-sourced control ever
  "stops working" after this, check whether the physical controller was swapped
  before assuming the Arduino/CAN logic broke** — the property name is tied to the
  exact device string SimHub sees, which changes with the hardware.
  **Resolved differently than planned (2026-09-19):** user decided not to hunt for
  a separate physical "MENU"-labeled button - instead repurposed **B02 itself**
  (the "MODE" button) to drive the MID/menu long-press-reset function, dropping the
  drive-mode-cycle binding on it entirely for now ("mod değiştirme için sonra başka
  tuş buluruz" - find a different button for drive-mode later). Implementation:
  `simhub/custom_protocol.txt` field 52 (`dmode`) reverted to the literal `'0'` placeholder it
  had before this session - the `dmode`/`prevDmode`/`driveModes[]` cycling code in
  `SHCustomProtocol.h`'s `read()` was left completely intact (not deleted), it just
  has no live input right now and is dormant until a new button is wired to field
  52. Field 36 (`braketemp`, previously dead/unused, same pattern as `showLights`
  above) now carries B02's raw held state instead
  (`if(isnull([InputStatus.JoystickPlugin.R3_Racing_Wheel_and_Pedals_B02],0)>0,1,0)`).
  In `SHCustomProtocol.h`, the `0x1EE` send (previously gated only on
  `menuButtonHeld`, the dash D4 button's flag) is now
  `(menuButtonHeld == 1 || braketemp == 1)` - both the dash button and the wheel
  button drive the same CAN signal, neither excludes the other. **Confirmed live
  working (2026-09-19):** holding B02 while on the average-consumption/range MID
  page resets it, same as D4. **If a drive-mode wheel button is wired up later,**
  reuse field 52 (still parses into `dmode`, logic untouched) with a new button's
  `[InputStatus.JoystickPlugin...]` reference — don't forget the `InputStatus.`
  prefix (see the root-cause entry right below this one).

- **`simhub/custom_protocol.txt` had drifted again — resynced (2026-09-19), same class of issue
  as the 2026-08-29 drift entry above.** User had edited the live SimHub formula
  directly (again) without copying it back to this repo file. Two substantive
  changes found in the user's live version that predate/are unrelated to today's
  B02 wiring, both carried forward into the resynced repo file: (1) field 2 (RPM)
  now scaled `*1.2` (`round(isnull([...Rpms],0)*1.2,0)`) — reason/origin not
  discussed this session, presumably a manual tweak to compensate for the
  documented RPM under/over-reading issues (see "RPM tach message" entry) but not
  confirmed; (2) most ETS2-specific fields got re-wrapped in
  `if([DataCorePlugin.CurrentGame]=='ETS2', <original>, 0)` guards — this reverses
  the 2026-08-18 decision documented below ("customprotocol structure") to drop
  per-game branching since the user doesn't play BeamNG; not discussed this
  session either, so unclear if intentional or a leftover from experimentation.
  **If BeamNG/game-switching behavior comes up again, check this guard is still
  there and intentional before assuming it's dead code.** Field 52 in the user's
  pasted version still had the stale G29 property — the B02 fix above was applied
  on top of this resync, not against the old repo copy. **Same standing workflow
  reminder applies: this repo file is only accurate until the next un-synced
  in-SimHub edit** — ask the user to paste back their live formula whenever
  behavior doesn't match what this file says.

- **Backlight fix (2026-09-19): reverted to headlights-only, needed an explicit
  off-send, confirmed working.** Following on from the 2026-08-28 entry above (which
  tied backlight to ignition instead of headlights, per that day's request) — user
  now wants the opposite: panel dark whenever headlights are off, even with ignition
  on. Reverted the `0x202` condition from `EngineIgnitionOn == 1` back to `light ==
  1`. **First attempt only wrapped the send in `if (light == 1) { ... }` with no
  `else` — confirmed live this left the panel stuck at the last brightness it ever
  received when `light` went back to 0, because the cluster does not dim on its own
  just because the message stops arriving, it holds the last value.** Fixed by
  adding an explicit `else` branch sending brightness `0` when `light != 1`. Also
  reconfirmed the general "reflash doesn't reset the cluster" rule from the
  parking-brake entry below applied here too (panel briefly stayed lit after a
  reflash until the cluster's own 12V was cycled). Confirmed live working correctly
  in both directions after this fix. No distinct "parking lights" signal exists in
  this protocol (only `light`/BeamLow) — user confirmed ETS2 already flags its
  parking-lights indicator together with headlights, so this wasn't needed
  separately; if that's ever wrong, would need a `LightsValues.Parking`-style
  property found live in SimHub (same method as always) and OR'd into the
  condition.
- **Cluster text color (white → orange) switching when headlights turn on — not
  independently controllable, same category as the other undocumented cosmetic
  cluster behaviors (2026-09-19).** User asked whether the dashboard's white text
  could stay white instead of switching to orange/amber when headlights are on.
  Checked the reference project specifically for this (fetched
  `BMWFSeriesCluster.cpp` fresh) — no separate day/night or text-color CAN message
  exists there either; the only relevant messages are `sendBacklightBrightness()`
  (`0x202`, intensity only) and `sendLights()` (`0x21A`, the same message this
  project already sends for low/high beam and fog indicators). This color switch is
  almost certainly the cluster's own hardcoded firmware response to the same
  "lights on" signal in `0x21A` that must be sent correctly for the beam/fog
  indicators to work — there is no known way to decouple them, sending correct
  light-on status necessarily also flips the display theme. Matches how a real BMW
  behaves (night theme when headlights on, to cut glare) so this is presumably
  intentional cluster design, not a bug. Would need real CAN sniffing on a donor car
  to find an independent control, if one even exists - not attempted, same
  "genuinely undocumented" bucket as fuel-sweep speed, average consumption, and
  cruise target speed above. Nothing changed in code for this one, informational
  only.

- **Fuel needle slow-fill-on-refuel — reconfirmed as pure cluster hardware damping,
  not a data/software issue (2026-09-19).** User asked again about the needle
  rising just as slowly when refueling to full as it normally falls while driving
  (same underlying issue as the extensively-documented "fuel-sweep-speed" entries
  above, first investigated 2026-08-20). This session added a concrete new data
  point that wasn't checked before: watched `DataCorePlugin.GameData.FuelPercent`
  directly in SimHub's live Available Properties view while refueling in ETS2 -
  confirmed it jumps **instantly** (91 -> 99.9 in one update, not gradually). This
  rules out both game-side refuel-animation ramping and any SimHub-side smoothing
  as the cause - the raw value we read and immediately forward via `0x349` is
  never slow. The slowness is 100% downstream of that, inside the cluster's own
  needle-motor drive, exactly as the 2026-08-20/2026-08-28 entries already
  concluded. Also tried a real in-game ignition off/on cycle after refueling
  (manually, in ETS2, not the untested Arduino-side auto-pulse idea floated in the
  2026-09-02 entry) hoping for the documented fast-sweep-on-ignition-transition
  behavior - user reports it still didn't jump cleanly to the correct final value,
  only caught up faster to wherever the needle had already slowly crept to by that
  point. **Decided (2026-09-19) to leave this as-is, matching the original
  2026-08-20 decision** - no further attempts planned; the gauge does eventually
  settle on the correct value on its own, just not quickly. If this comes up again,
  don't re-suspect the data path - re-read this entry and the ones above it first,
  the data path is now conclusively cleared.
- **New data point: the needle's fill speed is NOT uniformly slow across the whole
  range — it's fast up to roughly 75%, then slows for the remainder (2026-09-21).**
  User refueled from a near-empty tank (physical needle at "1 diş kalmış", i.e. one
  bar/tick left) to full. Observed on the real physical cluster needle itself (not
  SimHub's property browser, not the in-game HUD): the needle swept quickly up to
  about 75%, then visibly slowed down for the last ~25%. This refines, doesn't
  contradict, the 2026-09-19 entry above (which only established the raw
  `FuelPercent` data jumps instantly, not that the needle itself moves at one
  constant damped rate) — it suggests the cluster's internal needle-motor damping
  isn't a flat slow crawl over the whole delta, there may be a nonlinear
  acceleration/deceleration curve (fast start, slow approach to the target) baked
  into the cluster's own gauge-motor firmware, possibly similar to a real
  spring-and-damper needle physically settling into place. Still no known CAN
  message controls this — same "cosmetic-only, cluster-side, not fixable from the
  Arduino" conclusion as every other entry in this fuel-sweep saga. If this comes up
  again, get a specific number for how long the fast phase vs. slow phase each took
  and whether the 75% breakpoint holds for other starting/ending fuel levels (e.g.
  does refueling from 50% to 100% also fast-sweep up to ~75% then slow, or is 75%
  actually just "close to the target" rather than a fixed percentage) before
  guessing further.
- **Follow-up (2026-09-22): the ~75% breakpoint is NOT a fixed percentage — it
  moves with how empty the tank starts.** User refueled from a genuinely empty
  tank (0%, not "1 diş kalmış" like the 2026-09-21 case) to full, and this time
  the fast phase carried past 75%, filling noticeably more of the range quickly
  before any slowdown was noticeable. Combined with the 2026-09-21 data point,
  this confirms the 75% figure was specific to that day's starting level, not a
  constant — supports the "spring-and-damper settling" theory over a flat
  fast/slow split: the needle likely moves at a roughly constant fast speed
  from wherever it starts, and only decelerates in the final stretch as it
  nears whatever the target actually is, so a bigger total delta (starting from
  true 0%) means a proportionally bigger fast-phase before the same kind of
  end-of-travel deceleration kicks in. Still cluster-motor-internal, still no
  CAN control over it — this only refines the shape of the existing
  "cosmetic-only" conclusion, doesn't reopen it. If refuel-fill-speed comes up
  again, record the exact starting % (not just "low" or "empty") so the
  deceleration-zone-width-vs-total-delta relationship can actually be plotted
  instead of compared qualitatively.
- **The Arduino-side "fake ignition pulse" idea (floated 2026-09-02, still untested
  as its own hack) turns out to be moot — ETS2 already forces a real ignition
  off/on cycle around every refuel (2026-09-21).** User clarified you cannot
  refuel in ETS2 with the ignition/contact on at all, so every single refuel
  already goes through a real ignition-off (parked, fueling) then ignition-on
  (driving off) transition, with no Arduino-side trickery needed to produce one.
  This is exactly the scenario the 2026-09-19 entry above already tested ("tried a
  real in-game ignition off/on cycle after refueling... didn't jump cleanly to the
  correct final value, only caught up faster") and the 2026-09-21 "fast to ~75%,
  then slow" entry immediately above this one is almost certainly a closer, more
  precise observation of that exact same natural ignition-on transition, not a
  different mechanism. **Conclusion: there is nothing left to try on the
  Arduino/software side for this** — the real ignition-on fast-sweep behavior
  already fires on every refuel as a side effect of how ETS2's refueling works,
  and it already demonstrably does NOT fully solve the slow-fill (only gets
  partway, ~75%, before slowing again). A dedicated fake-pulse feature would just
  be redundantly triggering a transition that's already happening for free. Don't
  suggest or build the fake-ignition-pulse feature again unless new evidence
  suggests the natural in-game transition and an Arduino-forced one behave
  differently (no reason to expect they would — CAN sees the same
  `EngineIgnitionOn` 0->1 edge either way).
- **Checked whether percent-vs-liters unit confusion could explain the slow-fill
  (2026-09-21) — ruled out, not the cause.** User asked whether the cluster
  receives the fuel level as a percent or in liters, wondering if something's
  being missed there. Traced the full path: `simhub/custom_protocol.txt` field 4 sends
  `DataCorePlugin.GameData.FuelPercent` as a plain 0-100 integer (already
  confirmed correctly scaled for ETS2, see the 282.08/400L -> 70.52% entry above).
  `SHCustomProtocol.h`'s `read()` stores that as `fuelpercentage` (0-100), then
  `Loop()` runs it through `multiMap<uint8_t>(fuelpercentage, {0,50,100},
  {37,18,4}, 3)` — a 3-point piecewise-linear lookup (from the `MultiMap.h`
  library) that converts the 0-100% value into the cluster's own internal raw
  fuel-quantity scale (37=empty..4=full, the reference project's calibration, not liters and not
  raw percent — this is a cluster-specific unit nobody has a name for beyond
  "whatever `0x349` expects"). That single fully-computed raw value is sent
  whole, in one `CAN.sendMsgBuf(0x349, ...)` call per `Loop()` iteration — there
  is no Arduino-side ramping, no partial/intermediate values are ever sent
  between the old and new fuel level. So there's no unit-mismatch or
  quantization step on our side that could produce a "fast then slow" animation
  — confirms (again, from a different angle than the 2026-09-19 instant-jump
  check) that 100% of the visible sweep behavior happens inside the cluster's
  own gauge hardware after it receives the already-complete target value. Don't
  re-suspect the percent/liters conversion path for this issue again.

- **Oil temp needle repurposed to show ETS2 brake air pressure — implemented
  2026-09-21, NOT YET CONFIRMED LIVE.** User asked whether the truck's brake
  (compressed-air) pressure could be shown on the cluster. This is a real BMW
  F30 passenger-car KOMBI, not a truck cluster - it has no physical air-pressure
  gauge, and all 7 known `0x5c0` warning-lamp codes (24, 34, 35, 36, 71, 77, 215
  - see the seatbelt/parking-brake entry above for the full list) are already
  spoken for by other functions, so there was no free lamp to light either.
  Asked the user how they wanted to trade this off; they chose to sacrifice the
  oil-temperature gauge and show air pressure *level* (needle position only, not
  a calibrated reading) on that needle instead, since it's an existing physical
  gauge with headroom to reuse - same "repurpose a currently-idle/low-value
  field or output" pattern already used for `braketemp` (menu long-press) and
  field 52 (`dmode`, wheel MODE button) elsewhere in this file.
  **Implementation:** `simhub/custom_protocol.txt` field 3 changed from
  `isnull([DataCorePlugin.GameData.OilTemperature],'0')` to
  `round(isnull([DataCorePlugin.GameRawData.TruckValues.CurrentValues.MotorValues.BrakeValues.AirPressure],0) * 1.333, 0)`
  - property name chosen by analogy to `MotorValues.BrakeValues.ParkingBrake`
  (field 23, confirmed working) since both should live under the same
  `BrakeValues` struct, but **this specific property has NOT been confirmed to
  exist in SimHub's live Available Properties browser** - unlike most other
  properties in this file, which were only trusted after being found there live
  (see the standing lesson about `JoystickPlugin.*`/`InputStatus.` prefix drift,
  and the earlier `WaterTemperature`-wrong-for-ETS2 mistake). **Before pasting
  this into SimHub, search Available Properties for `Pressure` while driving in
  ETS2 and confirm the exact property name and whether it reads a sensible live
  value** - if the name is wrong, the field will just silently read `0` (same
  failure mode as every other bad property guess in this project), not error
  loudly. The `*1.333` scale assumes a real psi range of roughly 0-150 (typical
  SCS/ETS2 default) mapped onto this field's existing 0-200 internal range
  (`Temp` in `SHCustomProtocol.h`) - untested assumption, adjust the multiplier
  once the user can see where the needle actually sits at known pressure levels
  (e.g. full pressure after startup vs. after a hard brake). No Arduino-side
  code change was needed - the existing 0-200 clamp, EMA smoothing, and `0x3f9`
  send in `SHCustomProtocol::Loop()` all work unchanged on whatever value lands
  in `Temp`, see the comments added there for detail. **Trade-off the user
  accepted:** the oil-temperature reading (added 2026-08-29, see "Oil
  temperature gauge" entry below) is now gone from the physical cluster -
  don't reintroduce real oil temp on this field without checking with the user
  first, since this decision was deliberate.
- **Field 3 reverted back to oil temperature, air pressure dropped (2026-09-22).**
  User decided to give up the brake-air-pressure repurposing above and go back to
  real oil temperature on the 0x3f9 needle, this time scaled `*1.5` (reason for the
  multiplier not discussed - presumably to make the needle sweep further across the
  dial's range than the raw 0-~110°C oil temp value would). `simhub/custom_protocol.txt` field 3
  changed from
  `round(isnull([DataCorePlugin.GameRawData.TruckValues.CurrentValues.MotorValues.BrakeValues.AirPressure],0) * 1.333, 0)`
  back to `round(isnull([DataCorePlugin.GameData.OilTemperature],0) * 1.5, 0)` -
  same `DataCorePlugin.GameData.OilTemperature` property confirmed live on
  2026-08-29 (see "Oil temperature gauge" entry below), so this isn't a new/unverified
  property, just the multiplier changed from implicit `*1` to `*1.5`. No Arduino-side
  change needed (same as the pressure repurposing, the 0-200 clamp/EMA/0x3f9 send in
  `SHCustomProtocol.h` works on whatever lands in `Temp` unchanged) - **but the `Temp
  -= 3` offset and `Temp > 200` clamp in `SHCustomProtocol.h`, and the comments right
  above them, still describe the air-pressure repurposing and are now stale**, should
  be re-worded next time that code is touched. **Needs the user to paste the updated
  `simhub/custom_protocol.txt` into SimHub and confirm the needle tracks oil temp again** (not
  yet confirmed live as of this edit). If air pressure is ever wanted back, this
  entry and the one above it have both formulas on record.
- **Field 34 (fuel consumption) changed from `AverageConsumption*100` to
  `AverageConsumption/10` (2026-09-22) - implemented, but CURRENTLY HAS NO EFFECT
  ON THE CLUSTER, see the dead-variable finding below.** User asked for the MID
  average-consumption display to be driven by "anlık tüketim" (instant/live
  consumption) instead of the average, scaled `/10`. **No separate
  instant-consumption SimHub property has ever been found or used in this
  project** - `DataCorePlugin.GameRawData.TruckValues.CurrentValues.DashboardValues.FuelValue.AverageConsumption`
  is the only fuel-consumption-rate property that has ever appeared anywhere in
  this repo's history (see the 2026-08-29 resync entry above) - implemented using
  that same property with the multiplier changed to `/10` (was `*100`).
  **Reasoning for `/10` (from the user, 2026-09-22): this is a real BMW passenger-car
  cluster with a display range calibrated for a car's fuel consumption, but ETS2 is
  a truck sim - truck consumption numbers run far higher than a car's, so the value
  needs scaling down to fit what a "standart araba clusteri" expects.** Keep this
  reasoning in mind if the scale ever needs retuning (e.g. `/10` turns out too
  aggressive or not enough once it's actually wired to something visible) - it's
  about matching a truck's consumption range to a car cluster's expected range, not
  an arbitrary guess.
  **Dead-variable finding (2026-09-22, important): field 34 parses into `throthel`
  in `SHCustomProtocol.h`'s `read()`, but `throthel` is never referenced anywhere
  else in the file** - grepped the whole repo to confirm. The `0x2C4`
  "range/consumption bar" message (the one that *does* visibly move smoothly on the
  cluster, fixed 2026-08-28 - see the CRC/counter entry above) sends a hardcoded
  static payload (`0x64,0x64,0x64,0x01,0xF1`) plus a rolling `count` byte and CRC -
  it does NOT read `throthel`/the SimHub consumption value at all. **So right now,
  no matter what formula or scale field 34 uses, it has zero visible effect on the
  physical cluster** - the smooth bar movement the user has observed is entirely a
  side effect of the unrelated 2026-08-28 counter/CRC fix, not of this consumption
  value. This matches the already-documented "no known CAN message for average
  consumption/range exists" conclusion in the "Range/consumption bar erratic" entry
  below - `throthel` was very likely an earlier attempt to wire this in that got
  orphaned when the bar's counter/CRC bug was fixed separately. **User's decision
  (2026-09-22): keep the `/10`-scaled formula in `simhub/custom_protocol.txt` as-is** (it's
  harmless, and ready if a real CAN path is ever found/built) rather than revert it
  to `*100` just because it's currently inert. **If a real numeric consumption
  display on the cluster is wanted later, that needs new Arduino-side work to
  actually wire `throthel` into some CAN message** (no known candidate message
  exists yet - would need fresh investigation/CAN sniffing, same category as the
  other genuinely-undocumented cluster features in this file) and would require a
  reflash; simply pasting a new `simhub/custom_protocol.txt` formula is NOT enough for that,
  unlike the field-3 oil-temperature change above which the existing Arduino code
  already forwards unchanged.

## Known broken / not yet solved

- **Check-engine MIL icon (the literal yellow engine-shaped lamp) — CAN code
  unknown (2026-08-29).** User has two visually distinct yellow warning icons on
  this cluster: a general triangle-with-`!` (already lights correctly - this is
  what `0x5c0` code 34 drives, confirmed via a live isolated test with ignition
  forced off) and a separate, literal engine-block-shaped MIL lamp that does **not**
  light no matter what. Checked the reference project: it also only labels
  code 34 as "check engine" with no distinction between a triangle icon and an
  engine-shaped icon - so either the reference project's own cluster renders code 34 as the literal
  engine icon (different cluster variant/firmware than this one) or there's a
  genuinely separate, undocumented code for the engine-shaped lamp on this specific
  cluster. All known `0x5c0` codes are now used up (24, 34, 35, 36, 71, 77, 215 -
  see the seatbelt/parking-brake entry above for the full list) - none of them is
  documented as a second/distinct engine icon. Finding the right code would need
  either real CAN sniffing on a running car, or blind trial-and-error testing
  candidate numbers on `0x5c0` (same category of problem as the clock/date and
  cruise-control speed above) - not attempted yet, decide with the user whether
  it's worth the time before starting.
  **Tried (2026-08-29) sending all 7 known codes (24,34,35,36,71,77,215) as ON
  simultaneously in one `Loop()` iteration to see all lamps at once - don't repeat
  this.** Result was not a clean "all lamps lit" picture: nothing lit as expected,
  the airbag icon flashed instead (a lamp none of these codes should touch), and
  only the parking-brake lamp (from code 71) stayed on. Sending several different
  `0x5c0` codes back-to-back in a tight burst seems to confuse the cluster's
  fault-code parser rather than displaying them all - reverted immediately, cluster
  needed a real 12V cycle afterward to clear the stuck/flashed state (same pattern
  as every other "stuck after manual testing" case above). If lamp codes need
  testing again, send **one candidate code at a time**, each as its own isolated
  test (matching how the seatbelt 71 vs 77 isolation was done successfully), not a
  batch.
- **Oil temperature gauge — fixed (2026-08-29), needs live confirmation after
  pasting into SimHub.** Showed lowest reading regardless of real temp.
  `simhub/custom_protocol.txt` field 3 fell back through `[OilTemperature]` (generic property,
  empty for ETS2) to
  `DataCorePlugin.GameRawData.TruckValues.CurrentValues.DashboardValues.WaterTemperature`
  (also confirmed empty for ETS2 earlier) — both wrong. Found the real live property
  the same way `FuelPercent` etc. were confirmed: user searched SimHub's property
  browser while driving (engine warmed up) and found
  `DataCorePlugin.GameData.OilTemperature` = a sensible live value (94). Field 3
  changed to `isnull([DataCorePlugin.GameData.OilTemperature],'0')`. **Needs the
  user to actually paste the updated `simhub/custom_protocol.txt` into SimHub's Custom Serial
  Device editor** (editing the repo file alone doesn't change what SimHub is
  running) and confirm the gauge moves. Note: field 38 (`WTemp`, feeds the
  `>=119` engine-overheat warning lamp, separate from this gauge) still uses the
  old broken `WaterTemperature` GameRawData path - not touched yet, revisit with
  the same property-browser method if the overheat warning is ever suspected of
  not working either.
- **Cruise control target/set speed** — only on/off works (green icon). The actual
  speed number never displays. Confirmed this is missing in the reference project too
  (`0x289` cruise message there is also on/off-only, no speed byte). Would need
  fresh CAN sniffing (same method as the trip button) correlated with ETS2's
  `CruiseControlSpeed`-type property, changing the set speed while watching for
  which CAN ID changes.
- **Ambient/outside temperature and date/time on the small MID/secondary screen** —
  completely unimplemented; `H` (date/time) is read from SimHub into a variable that
  is never sent anywhere. Confirmed undocumented in the reference project too. Would
  need CAN sniffing from scratch; nobody has published this for this cluster as far
  as we found.
- Sis farı (fog light) mapped to
  `TruckValues.CurrentValues.LightsValues.AuxFront` — added by analogy to the
  other confirmed `LightsValues.*` paths but **never actually verified live**.
  Treat as unconfirmed until tested.
- **Range/consumption bar erratic ("kafasına göre azalıyor") — likely fixed
  (2026-08-28), needs live confirmation.** User reported the on-screen range/fuel
  economy indicator didn't decrease in real time with actual driving, moved
  inconsistently instead. Root cause found by diffing our `0x2C4` "MPG bar" send
  against the reference project's `sendDistanceTravelled()`: **two bugs** — (1) CRC
  polynomial was `0xFF`, should be `0xC6` (wrong poly means the checksum only
  coincidentally matches what the cluster expects, so the cluster accepts/updates
  the bar inconsistently rather than every cycle — this exactly matches the old
  code comment "fuel consumption sometimes moves but not every time"); (2) byte[0]
  was `throthel | counter4Bit` (a nonsense OR of throttle position with the 4-bit
  rolling counter) instead of a clean incrementing counter — the reference project uses a plain
  `count` variable for this, which our sketch already maintains elsewhere in
  `Loop()` (0-0x76 wrapping) but wasn't using here. Fixed both. **Confirmed live
  (2026-08-28)** via an isolated test (temporarily overrode `fuelpercentage` in
  `Loop()` to drain 100->0 over 5 minutes using `millis()`, independent of
  SimHub/the game, then reverted after confirming) - the range/bar now decreases
  smoothly instead of jumping erratically. Fix holds.
  **Separate, NOT fixed:** the actual km number shown for a "full tank" (user saw
  ~282km, low for a real F30) is controlled by something else entirely - almost
  certainly the cluster's own internally-remembered average-consumption-rate
  assumption (likely stale, left over from its life in the real donor car), which
  combines with the raw fuel-quantity byte sent on `0x349` to produce the displayed
  range. No CAN message for "average consumption" or "set range" exists in this
  project or the reference project (confirmed absent). Do NOT "fix" this by changing
  `outFuelRange = {37,18,4}` - that's the correct, already-validated needle
  calibration from the reference project, unrelated to this. Getting the displayed km to
  numerically match ETS2's actual remaining range isn't realistically achievable
  with current knowledge either way: ETS2 is a truck sim (liters, truck consumption
  physics) with no natural common unit to a BMW passenger-car cluster's internal
  consumption assumption. The only way to actually control the displayed range
  number would be finding a real "average consumption"/"distance to empty" CAN
  message via fresh sniffing on a real car - same category of genuinely-undocumented
  problem as the clock/date and cruise-control target speed above.
  **Related, not yet touched:** the reference project shows CAN ID `0x2C4` is actually
  dual-purpose — the same ID *also* carries engine temperature via a completely
  different payload (`{0x3e, engineTemperature, 0x64,0x64,0x64,0x01,0xF1}`, CRC
  poly `0xB2`) sent from a different function (`sendBasicDriveInfo`). Our sketch
  never sends this second variant at all. This might be part of why the engine
  temp gauge shows lowest reading regardless of real temp (separately from the
  already-known field-3 WaterTemperature-always-0-for-ETS2 bug) — worth revisiting
  together next time the temp gauge comes up, but deliberately not added yet since
  it wasn't part of what was being fixed here and needs its own verification.
## customprotocol structure

As of 2026-08-18, `simhub/custom_protocol.txt` is **ETS2-only** — the user doesn't play BeamNG
much, so the BeamNG/ETS2 auto-detect branching (`isnull(...ParkingBrake)` per field)
was removed and every game-specific field now points straight at ETS2's
`TruckValues.*` properties. Fields with a genuinely cross-game generic SimHub
property (speed, RPM, gear, throttle, oil/water temp fallback chain, fuel %, ABS,
TC, brake temp) were left as the plain generic property, same as before. Former
BeamNG-only fields (engine damage, tire deflation, doors/hood/trunk, velocity/accel
axes, etc.) are now hardcoded `'0'` placeholders rather than removed outright, to
keep the field count/order at 54 matching the Arduino side.

SimHub itself has **no built-in per-game formula switching** for Custom Serial
Devices — the protocol formula pasted into its editor is global and applies
regardless of which game is running (confirmed: no such option in the SimHub wiki
for Custom Serial Devices). The old auto-detect approach existed to work around
that. If BeamNG support is ever wanted again, either restore per-field branching
(check `git log`/`ets2` tag history) or accept manually swapping the formula in
SimHub's editor when switching games.

**IMPORTANT - the repo's `simhub/custom_protocol.txt` file had drifted out of sync with what's
actually pasted into SimHub (discovered 2026-08-29).** At some point the user
edited the live formula directly inside SimHub without it ever being copied back
here - this repo file is only a source of truth if it's kept in sync by actually
re-pasting after every edit (see workflow note below). When this was discovered,
the *live* SimHub formula (not this repo's stale copy) turned out to have several
fields **hardcoded to `'0'` that earlier CLAUDE.md entries had called "confirmed
working"**: field 5 (`OilPressureWarning`), fields 16-18 (`ABSActive`/`TCActive`/
`TCLevel`), field 31 (fog light `AuxFront`), field 32 (`BrakesTemperatureAvg` -
replaced with a hardcoded literal `'20'`, not real data), field 36
(`BrakesTemperatureMax`), field 38 (`WaterTemperature`, which feeds the `>=119`
overheat warning lamp - meaning that lamp likely never fires currently), and field
39 (`Throttle`). **Don't trust old "confirmed working" claims for these specific
signals without re-verifying live** - they may describe an earlier, since-abandoned
version of the formula, not what's actually running now. Two fields also turned out
to use different (possibly more correct, unclear why/when changed) property paths
than this repo previously had: field 23 parking brake via
`TruckValues.CurrentValues.MotorValues.BrakeValues.ParkingBrake` (repo had
`DashboardValues.ParkingBrake`), field 34 fuel consumption via
`DashboardValues.FuelValue.AverageConsumption` scaled `*100` (repo had
`DashboardValues.FuelAverageConsumption` unscaled). The repo file was resynced to
match the actual live formula on 2026-08-29 (only the oil-temperature fix layered
on top, see the oil temperature entry above) - **this repo file is the current
source of truth again, but only until the next time someone edits directly in
SimHub without syncing back.** Workflow going forward: after editing this file and
having the user paste+save it in SimHub, that's the sync point - if the user ever
reports unexpected behavior that doesn't match what this file says, the first
thing to check is whether SimHub's live formula still actually matches this file
(ask the user to paste back what's currently in SimHub, like was done here).

While doing this cleanup, also picked up one real fix from the user's own tested
SimHub config (a `.txt` export they'd saved outside the repo): turn signal fields
now also fire when `LightsValues.HazardWarningLights` is true, not just the
matching blinker — hazards weren't lighting the dash arrows before.

## Distance-multiplier hack for range/consumption display (2026-09-23 session) — IN PROGRESS, real permanent odometer cost

Full saga, so a future session doesn't re-derive or repeat mistakes. Context: user
reported the MID average-consumption display locks to a sticky ~29.5 (units
unclear, presumably L/100km) that never comes back down once reached, making the
displayed range on a full ETS2 tank look absurdly low (~140km) for what should be
a large truck tank. Confirmed via the reference project reference (fetched fresh) that
no CAN message for setting/resetting average consumption or range exists there
either - matches this project's own pre-existing "Range/consumption bar erratic"
conclusion. Real BMW cluster likely computes this internally from fuel-level drop
(`0x349`) over distance traveled (`0x2BB`, `distanceTravelledCounter`).

**Key finding: the average-consumption ceiling (~29.5) happens even at the
original, correctly-calibrated `Speed*2.9` multiplier, regardless of how gently
the throttle is used** - user confirmed this live. This means the ~29.5 ceiling is
NOT caused by our distance calibration at all - it's either a genuine hard display
cap in the cluster's firmware, or a value baked into the cluster's memory from its
donor car's real life (per the pre-existing "Range/consumption bar erratic" entry's
theory), independent of anything we send. **Increasing the distance multiplier is
very unlikely to fix the average-consumption number itself** - this remains
unresolved and is probably a dead end, same category as every other
"genuinely undocumented, cluster-internal" limitation in this file.

**However, the separate range figure (in km) DOES appear to respond to the
distance multiplier**, even though average consumption doesn't - these seem to be
computed somewhat independently inside the cluster. Test sequence, `Speed*K`
multiplier vs. resulting range (each a single test drive, NOT controlled for
driving style/duration/starting fuel level between tests - see caveat below):

| Multiplier | Range shown | Notes |
|---|---|---|
| `*2.9` (original, correct) | ~140km | baseline, confirmed 2026-09-22 |
| `*29` (10x) | 174km | modest, ambiguous - see sticky-consumption note below |
| `*58` (20x) | 811km | **best result so far** |
| `*100` | 600km | worse than *58 despite bigger multiplier |
| `*150` | 595km | worse again, roughly flat with *100 |

**Non-monotonic - bigger multiplier does NOT mean bigger range past *58.** Working
theory: a bigger multiplier makes the odometer/distance counter (`distanceTravelledCounter`,
a 16-bit Arduino `int`) hit a real functional breaking point at a proportionally
LOWER real speed (see below) - past that speed the cluster appears to stop
accepting/counting distance entirely, so a very large multiplier means the "window"
of real distance data that actually gets through before freezing is shorter, not
longer, capping the total distance (and therefore range) the cluster ever sees
lower than a more moderate multiplier would produce. **Not proven, but consistent
with all data points so far.** As of this entry, reverted back to `*58` (the best
confirmed value) after testing *100 and *150 both being worse - if this is revisited,
try values between `*29` and `*58` (e.g. `*45`) to look for the actual peak, and
don't go above `*58` again without new evidence.

**Confirmed regression: above roughly (2030 ÷ multiplier) km/h, real speed, the
cluster stops counting distance/odometer entirely.** Empirically found at `*58`:
broke above ~35km/h (35×58=2030). This ~2030 figure is the working estimate for
this cluster's real per-CAN-message-tick plausibility/overflow limit for this
value - either a 16-bit (`int`) wraparound happening too fast, or the cluster's own
sanity filter rejecting per-tick deltas that large as physically impossible. At
`*100`, this predicts breaking around ~20km/h; at `*150`, even lower - i.e. **at
these multipliers, real odometer/distance counting is broken for virtually all
normal driving speeds, not just extreme ones.** User was told this explicitly and
**chose to accept it anyway** - explicitly does not care about real-world odometer
accuracy since this cluster will never be reinstalled in a real car. Don't
re-explain this tradeoff as if it's new information in a future session; it's
already been fully discussed and accepted.

**Separate, more fundamental sticky-latch finding (not solved, not chased further
this session): average consumption was observed to read a reasonable ~15 while
driving gently, then jump to a sticky 29.5 after hard acceleration - and NEVER come
back down afterward even with continued gentle driving.** A real average should
decay back down as more low-consumption distance dilutes an earlier spike; it not
coming back down at all suggests this might be a latch/fault-style stuck state,
the same class as this cluster's other well-documented "gets stuck until a real
12V power cycle" quirks (parking-brake lamp, backlight, etc.) - NOT necessarily a
pure distance/fuel ratio problem. A user-proposed test to isolate this (real 12V
power-cycle + a trip reset via D4 long-press + driving with ZERO hard acceleration
the entire time, at the safe `*2.9` baseline) was suggested but not carried out
before the user chose to keep escalating the distance multiplier instead - **this
test is still worth doing if this area is revisited**, since if gentle-only driving
at the safe baseline still reliably produces a reasonable, non-stuck consumption
number, that would mean the whole multiplier-hacking approach was solving the wrong
problem the entire time (a TIR launching from a stop under load will basically
always trigger hard-acceleration-style fuel bursts, so if THAT is what causes the
latch, no multiplier choice fixes it - the real "fix", if any, would be about
avoiding/masking the trigger condition itself, not the distance calibration).

**A second, safer-in-principle mechanism was discussed but only partially tried:**
a flat, real-speed-independent increment (e.g. `distanceTravelledCounter += (Speed
> 0) ? K : 0` for some constant `K`) would never scale up with real driving speed,
so it could never cross the ~2030-per-tick breaking threshold no matter how fast
you actually drive - fully sidestepping the high-speed regression. User asked to
combine this with a speed-scaled term instead of using it standalone (`Speed*20 +
((Speed>0)?30:0)`, tried briefly as round 2 above, superseded by round 3's `*100`
pure-speed-scaled approach) - **the pure flat-rate-only approach (no speed-scaling
at all) was never actually tested on its own**, and remains the one approach in
this whole saga with no known real-speed breaking point, if a genuinely
regression-free way to inflate range is wanted later.

**Also note (2026-09-22, from the original saga above this section): the real
permanent odometer (441,614 km at the start of this session) has been inflated by
every multiplier test so far (`*29`, `*58`, `*100`, `*150`, each some indeterminate
amount depending on how far was driven during each test) - there is no known way to
undo this, reverting the multiplier only stops further inflation from that point
on. User has explicitly and repeatedly accepted this cost.**

Current code state as of this entry: `distanceTravelledCounter += Speed*58;` in
`SHCustomProtocol.h`'s `Loop()` (0x2BB block), compiled and uploaded to the board
(port drifted again this session: was COM19, now **COM25** - check CH340 port
fresh each time per the existing port-drift note elsewhere in this file, don't
assume COM25 is permanent either). **Not yet confirmed live whether *58 reliably
reproduces 811km again on a fresh test** (the non-monotonic results above suggest
test-to-test variance is real, not just multiplier-driven) - next session/test,
get a fresh reading before assuming 811km repeats exactly.

**Side effect discovered mid-session: the fuel gauge appeared "stuck at empty/0"
after this session's repeated rapid reflash + SimHub close/reopen cycles.** Root
cause: the old fuel-empty-test window (`fuelEmptyTestActive`/`inFuelEmptyTestWindow`,
part of the pre-existing fuel-sweep saga's "v1" version, see the entry above this
section) re-arms on every `EngineIgnitionOn` 0->1 edge and pins the fuel gauge to
raw-empty (37) for 2.5s each time - today's frequent reflashing/reconnecting kept
re-triggering it, and combined with this gauge's already-known heavy hardware
damping (real fills take minutes to visibly show), it looked stuck far longer than
2.5s. **Removed entirely at user's request (2026-09-23)** rather than kept as a
low-value, confusion-prone feature - the whole v1-v5 "force empty to help the
needle reach full" mechanism from the earlier saga is now gone from
`SHCustomProtocol.h`. Fuel now always sends the plain, real `multiMap()`-computed
value with no forced window at all. If the "needs 1-2 manual ignition cycles to
reach a full fill" behavior is missed later, that's this removal - it was already
flagged as "likely coincidental, not reliably reproducible" before being removed,
so this is not expected to be a meaningful loss.

**Confirmed side effect (2026-09-23): while the fuel gauge is slowly climbing back
from the empty raw value it got pinned to during today's testing, the cluster
shows a persistent low-fuel/range warning (menzil ~7km) the entire time.** This
matches - and confirms - the "watch for a spurious low-fuel warning lamp/chime on
ignition-on even with a full tank" caution that used to be in the old fuel-window
comment before it was removed above. Expected to clear on its own once the fuel
needle finishes its slow climb to the real value (est. ~15 minutes this time,
consistent with the gauge's well-documented heavy damping elsewhere in this file) -
no code action taken, don't try to force-clear this, forcing things on this
cluster tends to leave it in a worse stuck state per the many other entries in this
file about that.

## Brake air pressure critical warning lamp (2026-09-23) — property confirmed, lamp trigger not yet confirmed live

User wanted a yellow warning lamp when ETS2's brake air pressure gets critically
low (they'd seen the parking-brake-area lamp flash yellow on cluster startup and
knew it could show red when the handbrake is engaged, and wanted the yellow state
repurposed for this). Used `0x5c0` fault code **24** ("park brake error - yellow"
per the reference project's documented code list, see the seatbelt/parking-brake entry
higher in this file) - this code was completely unused anywhere in this project
until now, and is distinct from code 71 ("park brake error - red", permanently
disabled) and from the real handbrake-engaged light on CAN `0x34F`.

Implementation: `simhub/custom_protocol.txt` field 21 (landed there, not field 16 as first
planned - always verify with a fresh `Read` after an `Edit` on this file rather
than assuming a field number, the file has many near-identical `'0' + ';' +`
lines and a match can land on an unexpected one) now sends raw
`DataCorePlugin.GameRawData.TruckValues.CurrentValues.MotorValues.BrakeValues.AirPressure`
(no scaling this time, unlike the earlier field-3 `*1.333` attempt - we want the
real psi-ish number for a threshold comparison). This parses into `acc_lightstage`
in `SHCustomProtocol.h` (confirmed dead/otherwise-unreferenced before this change,
same repurposing pattern as `braketemp`/field 52 elsewhere in this file - variable
name kept as-is per that established convention, only its meaning changed).
`SHCustomProtocol.h`'s `Loop()` now sends 0x5c0 code 24 ON when
`acc_lightstage > 0 && acc_lightstage < 80`, OFF otherwise - the `> 0` guard is
deliberate: if the `AirPressure` property were ever wrong/missing, `isnull()`
defaults it to 0, and without the guard that would make the lamp permanently ON
(since 0 < 80) instead of off, which would be a much more confusing failure mode.

**Property CONFIRMED live (2026-09-23), unlike the earlier field-3 pressure-needle
attempt which was never verified**: user already reads this exact
`MotorValues.BrakeValues.AirPressure` property live as a level indicator on their
separate button box, and reported it showing 117 at the time - so this is real,
working ETS2 telemetry, not a guessed/dead property. **80 is the user's own chosen
threshold** (not a guess, unlike the initial 65 placeholder this replaced) - if it
ever needs retuning, that's a deliberate choice to revisit with the user, not a
guess to second-guess. Compiled and uploaded to the board (COM25). **Lamp
trigger behavior itself not yet confirmed live** (i.e. does code 24 actually show
as a yellow lamp on this specific cluster the way the reference project says it
should) - test by watching the lamp while braking down toward/below 80psi.
**User still needs to paste the updated `simhub/custom_protocol.txt` into SimHub's Custom
Serial Device editor** - editing the repo file alone doesn't change what SimHub
sends.

## Clock/date and cruise-control set-speed — E90 DBC experiment (2026-09-23), reverted after breaking an unrelated working feature

Two long-standing "Known broken/not yet solved" gaps in this file (ambient
clock/date on the MID screen, and the cruise control target-speed number) had no
known CAN message in this project or the reference project. Found a
candidate source instead of giving up: a community-maintained BMW **E90** (not
F30!) DBC file on GitHub - a public BMW E90 DBC file.
**E90 is an older BMW generation than this cluster's F30** - BMW's own
documentation notes the KOMBI/Gateway CAN protocol differs across generations, so
treat everything below as a real, unconfirmed experiment, not a verified fix -
same epistemic status as every other "found a candidate ID, haven't tested it yet"
entry in this file, just sourced from a different generation's DBC instead of
from guessing blind.

**Cruise control set speed** - CAN ID `0x193` (message `DCC_193` in the DBC),
signal `CruiseControlSetpoint` at byte 1, linear scale 1 with offset -2 (i.e. the
raw byte value = target km/h + 2). No checksum/counter byte documented for this
message. Implemented in `SHCustomProtocol.h`'s `Loop()` right after the existing
cruise on/off block (`0x289`) - sends `{0x00, oil_warn, 0xFF×6}` where `oil_warn`
(customprotocol field 11, previously dead - variable **type changed from
`String` to `int`**, was never referenced elsewhere so this was safe) carries
`DashboardValues.CruiseControlSpeed + 2`. **Property name fixed (2026-09-23): the plain guess `DashboardValues.CruiseControlSpeed`
errored in SimHub** - it's a compound object, not a plain number. User found the
real live sub-properties via Available Properties:
`DashboardValues.CruiseControlSpeed.Kph` and `.CruiseControlSpeed.Value` both
exist. Switched to `.Kph` since it's already in the unit this formula assumes
(km/h) - `.Value` might be a different unit (raw SCS unit, m/s, etc.), untested,
switch to it only if `.Kph` turns out wrong once live. This property is now
confirmed to exist (unlike before) - only the CAN ID/byte-layout generation-gap
risk remains as the open unknown for this specific feature.

**Clock/date on the MID screen** - CAN ID `0x2F8` (message `DATE_2F8`), byte
layout: byte0=hour(0-23), byte1=minute, byte2=second, byte3=day, byte4=high
nibble is month/low nibble always `0xF` (`(month<<4)|0x0F`), bytes5-6=year as a
16-bit little-endian value, byte7 undocumented (sent as `0xFF` filler). No
checksum/counter documented either. `DataCorePlugin.CurrentDateTime` is the PC's
real wall-clock time (not simulated in-game time) - extracted into 6 separate
`simhub/custom_protocol.txt` fields using SimHub's `format()` function with .NET-style date
specifiers (`'HH'`,`'mm'`,`'ss'`,`'dd'`,`'MM'`,`'yyyy'`). **This particular use of
`format()` on a DateTime-typed property is unconfirmed** - every other `format()`
call in this file formats a plain number (e.g. field 1's speed), not a date - if
the clock shows garbage, check each of these 6 formulas individually in SimHub's
Ncalc Tester before assuming the Arduino-side byte packing is wrong. Field
mapping (customprotocol field -> Arduino variable -> DBC signal): 35 (`H`) ->
hour, 16 (`absWarning`) -> minute, 18 (`dscSwitch`) -> second, 20
(`pcars_mcarflags`) -> day, 22 (`acc_flashlight`) -> month, 6
(`checkEngine_ETS`, **type changed from `String` to `int`**, was never
referenced elsewhere so this was safe) -> year. All were previously-dead fields,
confirmed unused elsewhere before repurposing (same method as the earlier
air-pressure-lamp and oil-temperature repurposings in this file - grep for the
variable name first, only repurpose if it's genuinely just declaration+parse).

**RESULT (2026-09-23): both features tested live, neither works - root cause
narrowed down to the CAN IDs themselves, not the data feeding them.** Sequence
followed: (1) `CruiseControlSpeed` alone errored in SimHub - fixed by switching
to the real sub-property `CruiseControlSpeed.Kph` (also `.Value` exists,
untried, unit unconfirmed) found live via Available Properties; (2) confirmed
all other gauges (speed, RPM, etc.) kept working normally throughout, ruling out
a broken/error-halted `simhub/custom_protocol.txt` string as the cause; (3) directly tested
the two clock formulas in SimHub's Ncalc Tester and compared against the real
time: `format([DataCorePlugin.CurrentDateTime],'HH')` returned `02`,
`format([...],'yyyy')` returned `2026`, and the user confirmed the real time was
**02:45** at that moment - both values are correct, proving `format()` works
correctly on the `CurrentDateTime` DateTime property (the one thing flagged
above as unconfirmed). **With the SimHub/data side now positively confirmed
correct, the only remaining explanation is the E90-vs-F30 generation gap in the
CAN IDs/layouts themselves (0x193, 0x2F8)** - this specific F30 cluster
silently ignores both messages, consistent with this cluster's well-established
pattern of silently discarding CAN data it doesn't recognize/expect rather than
erroring (same behavior seen with the out-of-range fuel test in the older
fuel-sweep saga elsewhere in this file). **Conclusion: this E90-DBC-based
approach is a dead end for this cluster generation.** ~~Both features remain
unimplemented from the user's perspective (code is harmless dead weight now -
not reverted, since sending these two extra CAN messages costs nothing and
doesn't interfere with anything else, but don't expect them to do anything).~~
**CORRECTION (2026-09-23): that "harmless dead weight" assumption was WRONG.**

**Checksum-guessing attempt and real-world root-cause research.** Before
concluding, tried adding a CRC8+alive-counter frame structure (matching this
cluster's own established convention for every other working message) with a
guessed seed (`0x82`, reused from the 0x289 cruise on/off message, arbitrary)
to both 0x193 and 0x2F8. Also did real research instead of guessing further:
found a documented case (Project Gus, F-series gear selector reverse
engineering blog) of the ACTUAL checksum algorithm BMW uses on this generation
- CRC-8/SAE-J1850 (poly `0x1D`, init `0x00`) with a **per-message XOR-out
constant that is NOT derivable from the CAN ID by any formula** - confirmed by
checking for an arithmetic relationship ourselves too (found none). The only
known way to determine a message's correct XOR-out is analyzing multiple real
captured samples from an actual running car with a tool like CrcBeagle - **if
real CAN sniffing on a donor F30 ever happens, this is the specific method/tool
to use, and 0x1D/0x00 is a confirmed-correct starting point for the base
algorithm**, don't re-derive from scratch. The 0x82 guess for 0x193/0x2F8 did
not produce any visible result either.

**Separately, found real information suggesting the whole "feed the clock via
live CAN" model was probably wrong from the start.** A BMW F30 forum thread
confirms the actual instrument cluster clock is a **local, independent
real-time clock the driver sets ONCE via the cluster's own menu** (physically:
push the turn-signal stalk to cycle the cluster's menu until a "Clock - Set"
page appears, then use the stalk's button to set hour/minute) - it is NOT fed
a live time value by any CAN message during normal operation. This matches a
live observation on this exact hardware: **after a real 12V power-cycle, this
cluster itself displays a "set date and time" prompt**, confirming it has its
own persistent RTC needing local setup, not external CAN feeding. Tried
reaching a "Clock/Set" page via the existing MID-cycle button (D4/`0x1EE`,
short press) - user reports it just loops back to the start, no clock page
found that way; also tried a long-press specifically while the "set date and
time" prompt was showing - also no effect. **Not resolved**: either this
particular cluster variant doesn't expose clock-set through the MID-cycle
button at all (maybe needs a genuinely different physical control this bench
setup doesn't have wired, e.g. a real steering-wheel SET button distinct from
MENU), or a different press pattern/timing is needed that hasn't been tried
yet. Worth revisiting with fresh button-sequence experimentation, but this
reframes the goal entirely: if solved, it would be a **local one-time cluster
setting**, not something `simhub/custom_protocol.txt`/SimHub feeds live - the whole
`DataCorePlugin.CurrentDateTime` formula work in this section may end up
unnecessary regardless of the CAN ID problem.

**REGRESSION FOUND AND FIXED (2026-09-23): these two experimental messages
were NOT harmless.** User reported the previously-confirmed-working
brake-air-pressure yellow lamp (0x5c0 code 24, see the entry above this one)
**stopped working** shortly after 0x193/0x2F8 were added. Standing lesson this
project already knew abstractly but hadn't hit concretely until now: **a
guessed CAN ID from a different generation isn't "safe to try, worst case it's
silently ignored"** - it can coincidentally correspond to something else
entirely on this generation's real bus, and an unexpected payload on that ID
could plausibly confuse a real function (this cluster's `0x5c0` fault-lamp
parser is already independently known to be fragile/get-confused-easily, see
the "sending several 0x5c0 codes in a tight burst" entry elsewhere in this
file). **Both 0x193 and 0x2F8 sends have been commented out** (not deleted -
see the `/* ... */` blocks in `SHCustomProtocol.h` right after the 0x289
cruise on/off send) specifically to test whether removing them restores the
air-pressure lamp - compiled and uploaded to the board. **Not yet confirmed
live whether removing them actually fixes the lamp** - if the lamp is still
broken after this revert alone, next try a real 12V power-cycle of the cluster
itself (not just an Arduino reflash) before assuming the revert didn't work,
matching this project's many other "stuck state needs real power-cycle, not
just reflash" precedents. **Standing lesson for future sessions: don't send an
untested/guessed CAN ID from another platform/generation to this real cluster
without treating it as a genuine risk to currently-working features, not just
a "might not do anything" experiment** - test one guessed ID at a time when
possible, and if something else breaks shortly after adding a new guessed ID,
suspect that new ID first.

**If cruise set-speed is wanted again, real CAN sniffing on an actual
F30-generation car (not just consulting another generation's DBC) is the only
known path forward** - same category as MID average-consumption/range already
documented as genuinely undocumented elsewhere in this file. **If a clock is
wanted again, first fully exhaust the local-menu-button-sequence angle** (see
above) before ever considering another guessed CAN ID - it's the safer and, if
BMW's own design is followed here, probably the actually-correct approach.
Don't retry other E-series (E60, E39, etc.) DBC files for this cluster without
new reasoning - the generation-gap risk (both the "doesn't work" and the
newly-discovered "might interfere with something else" kind) applies
generally, not just to E90 specifically.

**RESOLUTION (2026-09-23): the air-pressure lamp regression above was a false
alarm - not a cluster/CAN problem at all, a stale SimHub paste.** After the
12V-power-cycle test also failed to fix it, asked the user to paste back
exactly what was currently live in SimHub's Custom Serial Device editor to
compare against this repo's `simhub/custom_protocol.txt`. **It was significantly out of
date** - missing field 21 (`AirPressure`, hardcoded `'0'` instead) and every
other field touched during this entire session's clock/cruise/date work
(fields 5, 11, 16, 18, 20, 22, 35 were all still their pre-session values,
field 3's oil-temperature and field 34's consumption fixes had made it through
but nothing after). **Root cause: repeatedly force-closing SimHub
(`Stop-Process -Name SimHubWPF -Force`) for Arduino uploads - per the user's
own standing authorization from earlier this session, see
`simcluster-simhub-autoclose.md` in the memory directory - likely discarded an
unsaved paste in the Custom Serial Device editor at some point**, silently
reverting SimHub back to an earlier saved state with no error or warning. This
means the air-pressure lamp's earlier "harika tekte çalıştı" success was real
at the time, but a subsequent auto-close (before the user had clicked
Save/Kaydet in SimHub) wiped that formula back out - the cluster and Arduino
code were never actually broken, and the entire 12V-power-cycle /
CAN-interference investigation above was chasing the wrong cause.
**Standing lesson, applies whenever closing SimHub for this project going
forward: after asking the user to paste `simhub/custom_protocol.txt` into SimHub, confirm
they clicked Save before treating that sync as done - and be aware that
autonomously force-closing SimHub for an upload can silently discard an
unsaved paste with no error, which can masquerade as a hardware/cluster bug
and send debugging in a completely wrong direction (as it did here).** If a
previously-working feature mysteriously stops working, checking whether the
live SimHub formula still actually matches this repo file should be an early
step, not a last resort.

**Root cause of the SimHub-save issue, found later this session: the Custom
Serial Device formula editor has no visible Save button and relies on some
auto-persist behavior tied to a clean app shutdown - `Stop-Process -Force`
skips that entirely.** Standing workflow going forward: don't force-kill
SimHub by default anymore - ask the user to close it via its own UI (X
button/tray exit) before an upload, and only force-kill if they say it's
already closed or normal closing isn't an option. This is also saved in this
session's memory file (`simcluster-simhub-autoclose.md`).

## End of 2026-09-23 session - state and open items

Board (COM25 as of this session, was COM19 before - check CH340 port fresh
each time) left in a known-safe, confirmed-compiling state:
- Distance multiplier reverted to original `Speed*2.9` (was left at the risky
  `*58` for a while after the multiplier-tuning saga - fixed at the very end of
  this session after the user noticed range showing "----" at highway speed).
- Fuel-empty-forcing window removed entirely (was causing confusing "stuck at
  empty" appearances after rapid reflashing).
- Oil temperature (field 3, `*1.5`) and fuel consumption (field 34, `/10`)
  scaling changes from earlier this session are live and working.
- Brake air pressure -> yellow lamp (0x5c0 code 24, threshold 80) is
  implemented and *should* work now that the stale-SimHub-save issue is fixed -
  **not yet re-confirmed live after that fix**, check this first next session.
- Cruise-control-set-speed (0x193) and clock/date (0x2F8) sends are commented
  out (disabled, not deleted) in `SHCustomProtocol.h` - unresolved, low
  priority, would need real CAN sniffing or the local-menu-button angle (see
  the dedicated section above) to make progress.
- Low-beam headlight icon (0x21A) is reverted to the original `{0x05, 0x00,
  0xf7}` baseline - **still broken** (shows parking-lights icon instead of a
  distinct low-beam icon; high beam at `{0x07, 0x00, 0xf7}` still confirmed
  working). 8 byte combinations tried live this session, none worked - see the
  dedicated section above for the full list before trying more blindly.
- Headlight "selector"/flash-to-pass function: not investigated this session -
  next step would be finding the right ETS2/SimHub property (search "flash" in
  Available Properties) before any CAN work.
- Engine damage -> literal engine-icon lamp: not implemented. User found the
  real ETS2 property (`DataCorePlugin.GameRawData.TruckValues.CurrentValues.DamageValues.Engine`)
  but no CAN fault-code number for the literal engine MIL icon was found
  anywhere (not in the reference project, not in the reference project's GitHub issues, not in
  general BMW fault-code research) - would need real CAN sniffing or careful
  one-at-a-time (never batched - see the seatbelt/parking-brake and "sending 7
  codes at once" entries elsewhere in this file for why) testing of unused
  candidate numbers.

**Priority for next session: re-confirm the brake-air-pressure lamp works now
that the SimHub-save root cause is fixed** - most other open items here are
genuinely hard (need real CAN sniffing) and lower-value to keep chasing blindly
without new information.

## Git

- Remote `origin` is upstream project on GitHub — the logged-in account
  (`fatihcesur`) does **not** have push access to it (403). Commits are local-only
  for now; user chose to leave it that way rather than fork/request access.
  Confirm with the user again before trying to push, no need to keep retrying.
- A checkpoint of the ETS2 fixes (signals, lights, fuel, button, RPM, gear,
  auto-detect customprotocol) was committed and tagged `ets2` — `git checkout ets2`
  or `git show ets2` to get back to that exact working state if something regresses.

## 2026-10-08 session - cluster "dead" on the bench, white LEDs, needles re-seated

**Cluster did not react at all.** Diagnosed with three standalone sketches (no SimHub):
- `tools/bench/alltest/alltest`: wakes the cluster, every lamp on, 100 km/h / 5000 rpm / 75 % fuel,
  prints `ok/err/TEC/REC` once a second at 115200.
- `tools/bench/candiag/candiag`: listen-only, tries 500k@8/16 MHz, 250k, 1M, 100k and lists IDs seen.
- `tools/bench/loopdiag/loopdiag`: MCP2515 internal loopback, then one normal-mode send burst.

Readings and what they meant:
- Every send failed with TEC=0 and REC≈128, even with the CAN wires removed; loopback 20/20 OK.
  Cause 1: the module's VCC wire was on the Nano's **RST** pin (next to 5V). The MCP2515 still
  answered over SPI (back-fed through the SPI pins), but the TJA1050 transceiver had 0 V. The
  same miswire also broke DTR auto-reset, so uploads failed with `not in sync`.
- After moving VCC to 5V (4.70 V measured): module alone = TEC=128 REC=0 (healthy, no ACK).
  With the USB-C extension plugged in: CAN-H 2.5 V, **CAN-L 1.5 V** -> bus stuck dominant,
  REC≈128 again, no frames from the cluster. Module alone measured H 2.5 / L 2.08 V.
  Cause 2: the USB-C breakout sockets (continuity was fine, so a leak/bad joint, not an open).
  After the user reworked the connection: ok rising, err=0, all lamps on. The user will clean
  (IPA) and resolder the breakouts properly; check L<->G and H<->G read OL in ohms mode.
- Rule of thumb: TEC=0 + REC≈128 = transceiver sees the bus stuck dominant (unpowered
  transceiver or CAN-L pulled low). TEC=128 + REC=0 = healthy module, nobody ACKing.

**Backlight always on**: new `backlightAlwaysOn = true` in `bmw_f30_cluster.ino` (was tied to the
game's headlights / parking lights).

**Hardware changes by the user**:
- Amber dial backlight LEDs replaced with warm-white LEDs from an LED strip (bench: white was
  brighter at the same current). Needles now white too; the redline segment stays red.
  Photos: `docs/images/2026-10-08/amber_leds_before.jpg`, `white_leds_after.jpg`.
- Needles pulled and re-seated. Check with alltest: 100 km/h read 102, 5000 rpm read 5100,
  fuel ~82 % at 75 % (still settling). Two-point test (40/200 km/h, 1000/6000 rpm) still to
  do: same % error at both ends -> change the 64.01 / 1.557 factors; same absolute error ->
  re-seat the needle. Real BMWs over-read speed ~2 % on purpose.
- PCB photo (`pcb_speedometer_cruise_marker.jpg`) shows the **cruise set-speed marker**: a
  black arm rotating around the speedometer motor with a green/orange bi-colour LED at its
  tip, fed by an orange flex to a small white connector. This is the hardware for the
  unsolved "cruise set speed" item. It was left out on reassembly; refit before hunting its
  CAN message with the camera.

**Own CRC-8**: `CRC8.h` (header-only, bitwise, poly 0x1D, init 0xFF, per-message XOR) replaces
the reference project's table-based `CRC8.cpp` in all three sketches. Verified identical on 200k random frames.
Frees 256 bytes of RAM (main firmware 624 -> 880 bytes free) and lets the project be MIT.

**GitHub**: published as the public repo `fatihcesur/bmw-f30-cluster-simhub` with a fresh
history and a cleaner layout (`firmware/`, `simhub/`, `docs/`, `tools/bench/`). The full
development history lives in the private repo `fatihcesur/bmw-f30-cluster-simhub-archive`.
VIN and cluster serial were redacted. Highlights and a check-control code photo atlas
(`docs/cc_atlas/`) were added from the local webcam captures.

**Assetto Corsa / other games (2026-10-08)**: `simhub/custom_protocol.txt` became game-aware.
ETS2 and ATS share the truck branch; every other game falls back to BeamNG raw `light_*`
OR SimHub common data (`TurnIndicatorLeft/Right`, `Handbrake` > 50 -> parking brake,
`PitLimiterOn` -> cruise icon) OR ACC `Graphics.LightsStage` (1 = lights, 2 = high beam).
Original AC exposes no headlight data. Untested in game: if a lamp stays dark (or the whole
formula errors), check the property types in SimHub while AC runs. The previous ETS2-only
formula is kept as `simhub/legacy/ets2_formula_2026-10-08.txt`.
