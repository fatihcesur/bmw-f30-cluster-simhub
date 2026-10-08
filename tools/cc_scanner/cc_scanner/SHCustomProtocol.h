#ifndef SHCUSTOMPROTOCOL_H
#define SHCUSTOMPROTOCOL_H

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>
#include "MultiMap.h"
#include "X9C10X.h" 
#include "CRC8.h"
#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)
#define loo8(x) ((int)(x)&0xff)
#define hii8(x) ((int)(x)>>8)
extern bool GRPMOND;
extern bool EDITEDSELUA;
extern bool checkengwhenoff;
extern int setlanguage;
extern int setcorf;
extern int setmpgl100orkml;
extern int setkmormiles;
extern int backlight;
extern int drivemod;
extern int menuButtonHeld;
extern int seatbeltBuckled;
extern int driveModes[];
extern const int driveModesCount;

const int spiCS = 10;  
const int intPin = 2; 

MCP_CAN CAN(spiCS);
CRC8 crc8Calculator;


// create varibles
int Speed = 0;
int RPM = 0;
int Temp = 140; // repurposed (2026-09-21): now carries brake air pressure, not oil temp - see Loop() below
int count = 0;
int count2 = 0;
String blinkerleft;
String blinkerright;
int cruise;
String parkingbreak;
int oil_warn = 0; // repurposed (2026-09-23): cruise control set speed, see the 0x193 send below
String batWarning = "FALSE";
String lights;
int turnLeft = 0;
int turnRight = 0;
int absWarning = 0;
int dscWarning = 0;
int dscSwitch = 0;
String checkEngine_PCARS = "0";
String Clusterlight = "False";
int pcars_mcarflags = 0;
int acc_lightstage = 0;
int acc_flashlight = 0;
int handbrake = 0;
int turnleft_fs = 0;
int turnright_fs = 0;
String cruise_fs;
String engine_fs;
String lights_fs;
String gear;
int showLights = 0;
int light = 0;
int highbeam = 0;
int fogg = 0;
int brakes = 0;
int EngineIgnitionOn = 0;
int throthel = 0;
int H = 0;
int braketemp = 0;
String Game = "ETS2";
int fuelpercentage = 0;
int oilpress = 0;
int checkEngine_ETS = 0; // repurposed (2026-09-23): clock year, see the 0x2F8 send below
int rightActive = 0;
int leftActive = 0;
int RPM2 = 0;
int handproc = 0;
int prevhand = 0;
int WTemp = 0;  
int counter4Bit = 0;
int dfl = 0;
int dfr = 0;
int drl = 0;
int drr = 0;
int hood = 0;
int trunk = 0;
int tpms = 0;
int sos = 0;
int dmode = 0;
int manual = 0;
int gas = 0;
int gmax = 0;
int runs = 0;
int runsg = 0;
int runsb = 0;
int tfr = 0;
int tfl = 0;
int trr = 0;
int trl = 0;
int tempomats = 0;
int idleRPM = 0;
int efficient = 0;
int distanceTravelledCounter = 0;
int selectedGear = 0;
int corf;
int mpgl100orkm;
int kmormiles;
int byte2;
int byte3;
int cv;
// ---- cc_scanner additions (tools/cc_scanner only, not in the real firmware) ----
int scanCode = -1;        // 0x5c0 check-control code currently forced ON, -1 = none
int scanOffCode = -1;     // code that still needs a few explicit OFF frames
int scanOffRepeats = 0;
uint8_t lightsOverride[3] = { 0x00, 0x00, 0xf7 };
bool lightsOverrideOn = false;
// fuel-used counter for 0x2C4 byte0 (experiment): grows every 100 ms by
// Speed * throthel * fuelK / 1e6, so the cluster's own fuel/distance maths sees a
// consumption proportional to throthel instead of the old per-Loop `count`.
// fuelK = 0 keeps the original `count` behaviour for comparison.
float fuelAccum = 0;
long fuelK = 0;
int fuelByte = 0;
// 0x2C4 layout experiment: c4hdr = 0 keeps the original frame; otherwise byte1 = c4hdr|counter4Bit,
// bytes2-7 = 0xFF (overridable with o 2c4), CRC over bytes 1-7 with seed c4seed
int c4hdr = 0;
long cntPeriod = 0;
float distMul = 1;
unsigned long ignBlipUntil = 0;
int r3f9 = 0;   // "v r3f9 1": send the reference project's 0x3F9 layout (overrides via id 0x3F8)   // "v blip <ms>": only the 0x12F byte says ignition off   // teach: multiplies distance + fuel counter steps (display speed unchanged)
unsigned int loopsPerCnt = 0;
int c4seed = 0xC6;   // 0: counter in byte0 (old); 1..5: 16-bit LE counter in bytes N,N+1, byte0 keeps `count`
// 0xF3 experiment: 0 = original double send (stale/whole-frame CRC), 1 = one frame, CRC over bytes 1-7 with seed crcSeedF3
int rpmMode = 0;
// fuel experiment: fuelRaw16 >= 0 sends this 16-bit value in 0x349 (higher = emptier),
// growing by fuelDrain raw units per 10 s while Speed > 0
float fuelRaw16 = -1;
int fuelDrain = 0;
int crcSeedF3 = 0x7A;
// byte overrides "o <id> <idx> <val>": applied to a payload right before its CRC/send
uint16_t ovId[6]; uint8_t ovIdx[6]; uint8_t ovVal[6]; uint8_t ovN = 0;
// Loop()'s own 0x5c0 sends go through here so they don't fight the code under test
void send5c0(uint8_t *f) {
  if (scanCode >= 0 && (f[1] | (f[2] << 8)) == scanCode) return;
  CAN.sendMsgBuf(0x5c0, 0, 8, f);
}
void applyOv(uint16_t id, unsigned char *buf, uint8_t len) {
  for (uint8_t i = 0; i < ovN; i++)
    if (ovId[i] == id && ovIdx[i] < len) buf[ovIdx[i]] = ovVal[i];
}
// CRC-8 Calculation Function only for RPM
uint8_t crc8(uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;  // Initial value
    uint8_t polynomial = 0x1D;  // Polynomial

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];  // XOR the current byte with the CRC

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {  // If the MSB is set
                crc = (crc << 1) ^ polynomial;  // Shift and XOR with the polynomial
            } else {
                crc <<= 1;  // Just shift left
            }
        }
    }

    return crc ^ 0x2C;  // Final XOR with 0x2C
}

class SHCustomProtocol {

public:

void Setup() { 
	    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        Serial.println("MCP2515 Initialized Successfully!");
        CAN.setMode(MCP_NORMAL);
    } else {
        Serial.println("Error Initializing MCP2515...");
        while (1);  
    }
    crc8Calculator.begin();
    pinMode(intPin, INPUT);
prevhand = 1;
}

void read() {
      Speed = FlowSerialReadStringUntil(';').toInt();                        
      RPM = FlowSerialReadStringUntil(';').toInt();
      Temp = FlowSerialReadStringUntil(';').toInt();
      fuelpercentage = FlowSerialReadStringUntil(';').toInt();
      oilpress = FlowSerialReadStringUntil(';').toInt();
      checkEngine_ETS = FlowSerialReadStringUntil(';').toInt();
      blinkerright = FlowSerialReadStringUntil(';');
      blinkerleft = FlowSerialReadStringUntil(';');
      cruise = FlowSerialReadStringUntil(';').toInt();
      parkingbreak = FlowSerialReadStringUntil(';');
      oil_warn = FlowSerialReadStringUntil(';').toInt();
      batWarning = FlowSerialReadStringUntil(';');
      lights = FlowSerialReadStringUntil(';');
      turnLeft = FlowSerialReadStringUntil(';').toInt();
      turnRight = FlowSerialReadStringUntil(';').toInt();
      absWarning = FlowSerialReadStringUntil(';').toInt();
      dscWarning = FlowSerialReadStringUntil(';').toInt();
      dscSwitch = FlowSerialReadStringUntil(';').toInt();
      checkEngine_PCARS = FlowSerialReadStringUntil(';');
      pcars_mcarflags = FlowSerialReadStringUntil(';').toInt();
      acc_lightstage = FlowSerialReadStringUntil(';').toInt();
      acc_flashlight =  FlowSerialReadStringUntil(';').toInt();
      handbrake =  FlowSerialReadStringUntil(';').toInt();
      turnleft_fs =  FlowSerialReadStringUntil(';').toInt();
      turnright_fs =  FlowSerialReadStringUntil(';').toInt();
      cruise_fs = FlowSerialReadStringUntil(';');
      engine_fs = FlowSerialReadStringUntil(';');
      lights_fs = FlowSerialReadStringUntil(';');
      gear = FlowSerialReadStringUntil(';');
      light = FlowSerialReadStringUntil(';').toInt();
      fogg = FlowSerialReadStringUntil(';').toInt();
      brakes = FlowSerialReadStringUntil(';').toInt();
      EngineIgnitionOn = FlowSerialReadStringUntil(';').toInt();
      throthel = FlowSerialReadStringUntil(';').toInt();
      H = FlowSerialReadStringUntil(';').toInt();
      braketemp = FlowSerialReadStringUntil(';').toInt();
      highbeam = FlowSerialReadStringUntil(';').toInt();
      WTemp = FlowSerialReadStringUntil(';').toInt();
      gas = FlowSerialReadStringUntil(';').toInt();
      idleRPM = FlowSerialReadStringUntil(';').toInt();
      tfl = FlowSerialReadStringUntil(';').toInt();
      tfr = FlowSerialReadStringUntil(';').toInt();
      trl = FlowSerialReadStringUntil(';').toInt();
      trr = FlowSerialReadStringUntil(';').toInt();
      tempomats = FlowSerialReadStringUntil(';').toInt();
      dfl = FlowSerialReadStringUntil(';').toInt();
      dfr = FlowSerialReadStringUntil(';').toInt();
      drl = FlowSerialReadStringUntil(';').toInt();
      drr = FlowSerialReadStringUntil(';').toInt();
      hood = FlowSerialReadStringUntil(';').toInt();
      trunk = FlowSerialReadStringUntil(';').toInt();
      dmode = FlowSerialReadStringUntil(';').toInt();
      // Wheel button (SimHub-sourced, field 52): raw pressed/not-pressed state (0/1),
      // mirrors GPIO Button 3 (BUTTON_PIN_3, pin 5) - cycles drivemod through
      // driveModes[] on the press edge only (0->1 transition), so holding the wheel
      // button down doesn't advance repeatedly.
      static int prevDmode = 0;
      if (dmode == 1 && prevDmode == 0) {
        int idx = 0;
        for (int i = 0; i < driveModesCount; i++) {
          if (driveModes[i] == drivemod) {
            idx = i;
            break;
          }
        }
        drivemod = driveModes[(idx + 1) % driveModesCount];
      }
      prevDmode = dmode;
      showLights = FlowSerialReadStringUntil(';').toInt();
      Game = FlowSerialReadStringUntil('\n');
}



// "P <hexid> <b0> ..": send this raw frame every 100 ms (like a real ECU), "P -" stops
long perId = -1; uint8_t perBuf[8]; uint8_t perLen = 0; unsigned long perLast = 0;
// "Q <idx>": low nibble of perBuf[idx] becomes an alive counter 0..14, "Q -" off
int8_t perCnt = -1; uint8_t perCntVal = 0;

void ScanSend() {
  if (perId >= 0 && millis() - perLast >= 100) {
    perLast = millis();
    if (perCnt >= 0 && perCnt < perLen) {
      perBuf[perCnt] = (perBuf[perCnt] & 0xF0) | perCntVal;
      perCntVal = (perCntVal + 1) % 15;
    }
    CAN.sendMsgBuf(perId, 0, perLen, perBuf);
  }
  if (scanOffCode >= 0 && scanOffRepeats > 0) {
    uint8_t off[] = { 0x40, (uint8_t)(scanOffCode & 0xFF), (uint8_t)(scanOffCode >> 8), 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
    CAN.sendMsgBuf(0x5c0, 0, 8, off);
    if (--scanOffRepeats == 0) scanOffCode = -1;
  }
  if (scanCode >= 0) {
    uint8_t on[] = { 0x40, (uint8_t)(scanCode & 0xFF), (uint8_t)(scanCode >> 8), 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
    CAN.sendMsgBuf(0x5c0, 0, 8, on);
  }
}

void Loop() {

// Welcome sweep: when ignition transitions off->on, briefly send max values on
// speed/RPM/temp gauges so the needles sweep to the end and back, like a real
// self-test. 2 second window, then real values resume. Fuel has its own separate
// window below - see the fuel-empty-test comment near the 0x349 send.
static int prevIgnitionState = 0;
static unsigned long sweepStartMs = 0;
static bool sweepActive = false;
// REMOVED (2026-09-23): the separate "force fuel gauge to empty for 2.5s after
// ignition-on" test window (fuelEmptyTestActive/inFuelEmptyTestWindow) that used
// to live here - see CLAUDE.md's fuel-sweep saga (v1-v5) for the full history.
// That v1 version was already flagged as "likely coincidental, not reliably
// reproducible" in the 2026-09-22 session conclusion, and this session's repeated
// rapid reflash/SimHub-reopen cycles kept re-triggering it (it re-arms on every
// ignition 0->1 edge), making the already-known-slow fuel needle appear "stuck at
// empty" for confusingly long stretches. Removed at the user's request rather
// than kept as a low-value, confusion-causing feature. Fuel now always sends its
// real computed value (see `fuelQuantityLiters` near the 0x349 send below) with
// no forced-empty window at all.
if (EngineIgnitionOn == 1 && prevIgnitionState == 0) {
  sweepActive = true;
  sweepStartMs = millis();
}
prevIgnitionState = EngineIgnitionOn;
bool inSweepWindow = sweepActive && (millis() - sweepStartMs < 2000);
if (sweepActive && !inSweepWindow) {
  sweepActive = false;
}

// counter4bite ++
counter4Bit++;
if (counter4Bit >= 14) { 
counter4Bit = 0; 
}
 
     
// Temp part
// (2026-09-21) `Temp` (customprotocol field 3) no longer carries real oil
// temperature - repurposed to show ETS2's brake air pressure on the oil-temp
// needle instead (this cluster has no dedicated air pressure gauge; user chose
// to sacrifice the oil-temp reading to see pressure LEVEL on this needle
// instead). customprotocol scales AirPressure (assumed ~0-150 psi) to this
// field's existing 0-200 range with `*1.333`; the 0-200 clamp and the 0x3f9
// send below are otherwise untouched, so needle position is just proportional
// pressure now, not a calibrated psi reading. The dial's printed numbers/units
// are for temperature and no longer mean anything literal - only relative
// needle position (low/high) is meaningful now.
// EMA smoothing below predates this repurposing (was for OilTemperature's wild
// jitter) but is harmless/plausibly still useful here to damp noise - revisit
// the 0.02 factor if the pressure needle reacts too slowly to real pressure
// drops (e.g. during hard/sustained braking).
static float smoothedTemp = 0;
smoothedTemp += (Temp - smoothedTemp) * 0.02;
Temp = (int)smoothedTemp;
if (Temp > 200) {
Temp = 200;
}
if(Temp > 0) { // -3 tweak was tuned for oil temp display, now just a ~1.5% offset on the pressure scale - harmless, not retuned
 Temp -= 3;
}


//ing part and Temp part
{
  // cntPeriod experiment: 0 = +1 every Loop (original), N = +1 every N ms, -1 = frozen
  static unsigned long lastCntMs = 0;
  if (cntPeriod == 0 || (cntPeriod > 0 && millis() - lastCntMs >= (unsigned long)cntPeriod)) {
    lastCntMs = millis();
    loopsPerCnt = 0;
    if(count == 0x77) {
    count = 0;
    }
    count +=1;
  }
  loopsPerCnt++;
}
// 0x8A = ignition on ("READY"/running), 0x8 = ignition off ("OFF") - per
// the reference project's sendIgnitionStatus(): "ignitionStatus = ignition ? 0x8A : 0x8".
// Previously hardcoded to 0x8A always, so the tach dial's OFF position never showed.
uint8_t ignitionStatus = (EngineIgnitionOn == 1 && (long)(millis() - ignBlipUntil) >= 0) ? 0x8A : 0x8;
        unsigned char ignitionWithoutCRC[] = { 0x80 | counter4Bit, ignitionStatus, 0xDD, 0xF1, 0x01, 0x30, 0x06 };
        applyOv(0x12F, ignitionWithoutCRC, 7);
        uint8_t crc = crc8Calculator.get_crc8(ignitionWithoutCRC, 7, 0x44);
        unsigned char ignitionWithCRC[] = { crc, ignitionWithoutCRC[0], ignitionWithoutCRC[1], ignitionWithoutCRC[2], ignitionWithoutCRC[3], ignitionWithoutCRC[4], ignitionWithoutCRC[5], ignitionWithoutCRC[6] };
    CAN.sendMsgBuf(0x12F, 0, 8, ignitionWithCRC);
unsigned char ingandtemp[8] = {0x0, count, count, 0x00, 0x00, count, count, count}; // ignition for F15 cluster

{
int tempForDisplay = inSweepWindow ? 200 : Temp;
ingandtemp[5] = int((0.983607*tempForDisplay) + 51.3169);
if (r3f9) {
  // the reference project layout: {CRC, 0x10|counter, 0x82, 0x4E, 0x7E, temp+50, 0x05, 0x89}, seed 0xF1
  unsigned char r[7] = { (uint8_t)(0x10 | counter4Bit), 0x82, 0x4E, 0x7E, (uint8_t)(tempForDisplay + 50), 0x05, 0x89 };
  applyOv(0x3F8, r, 7);
  unsigned char w[8] = { crc8Calculator.get_crc8(r, 7, 0xF1), r[0], r[1], r[2], r[3], r[4], r[5], r[6] };
  CAN.sendMsgBuf(0x3f9, 0, 8, w);
} else {
applyOv(0x3F9, ingandtemp, 8);
CAN.sendMsgBuf(0x3f9, 0, 8, ingandtemp);
}
}


//if water over 119 then activate engine over heated aleart
if(WTemp >= 119) {

 	      uint8_t engine_overheated[] = { 0x40, 39, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(engine_overheated);
} else {

 	      uint8_t engine_overheated1[] = { 0x40, 39, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(engine_overheated1);
}


// handbrake lamp only sent once because sending multible times cluster not happy
uint8_t handbrake_frame[2] = {0xFE, 0xFF};


  if(handbrake == 1){
    handbrake_frame[0] = 0xFE;
  }else{
    handbrake_frame[0] = 0xFD;
  }
  if (handbrake != prevhand) {
prevhand = handbrake;
handproc = 0;
}
  if(handproc == 0) {
  CAN.sendMsgBuf(0x34F, 0, 2, handbrake_frame);
  handproc = 1;
}


//remote MID/menu button (button 2, pin 4) — sent every cycle like the other
//level-based signals below, so the cluster sees a continuous signal while held
//(needed for its long-press-to-reset-trip-data behavior, not just page cycling)
//`braketemp` (customprotocol field 36, was a dead/unused field) repurposed
//(2026-09-19) to also carry the wheel's MODE button (B02) raw held state, ORed in
//here so both the dash D4 button and the wheel button drive the same behavior.
uint8_t menuButtonFrame[2] = { (menuButtonHeld == 1 || braketemp == 1) ? (uint8_t)76 : (uint8_t)0, 0xFF };
CAN.sendMsgBuf(0x1EE, 0, 2, menuButtonFrame);


//blinkers
 uint8_t blinkerStatus = (turnLeft == false && turnRight == false) ? 0x80 : (0x81 | turnLeft << 4 | turnRight << 5);
    unsigned char blinkersWithoutCRC[] = { blinkerStatus, 0xF0 };
    CAN.sendMsgBuf(0x1F6, 0, 2, blinkersWithoutCRC);
    
    
//Speed
	       if(Speed > 350) {
	Speed = 350;
}
uint16_t calculatedSpeed = inSweepWindow ? (uint16_t)(260 * 64.01) : (uint16_t)((double)Speed * 64.01); // 260 = dial's printed max
  unsigned char speedWithoutCRC[] = { 0xC0|counter4Bit, lo8(calculatedSpeed), hi8(calculatedSpeed), (Speed == 0 ? 0x81 : 0x91) };
  unsigned char speedWithCRC[] = { crc8Calculator.get_crc8(speedWithoutCRC, 4, 0xA9), speedWithoutCRC[0], speedWithoutCRC[1], speedWithoutCRC[2], speedWithoutCRC[3] };
  CAN.sendMsgBuf(0x1A1, 0, 5, speedWithCRC);	


//RPM
    if(RPM > 7500) {
	RPM = 7500;
}
if(GRPMOND == true) {
if (RPM < 1000 or RPM == 1000) {
RPM2 = map(RPM, 0, 7500, 0, 6200);
} else if (RPM < 2000 or RPM == 2000) {
RPM2 = map(RPM, 0, 7500, 0, 6000); 
} else if (RPM < 3000 or RPM == 3000) {
RPM2 = map(RPM, 0, 7500, 0, 5900);
} else if (RPM < 4000 or RPM == 4000) {
RPM2 = map(RPM, 0, 7500, 0, 5850); 
} else if (RPM < 5000 or RPM == 5000) {
RPM2 = map(RPM, 0, 7500, 0, 5850);	
} else if (RPM < 6000 or RPM == 6000) {
RPM2 = map(RPM, 0, 7500, 0, 5840); 	
} else if (RPM < 7000 or RPM == 7000) {
RPM2 = map(RPM, 0, 7500, 0, 5840); 
} else if (RPM < 7500 or RPM == 7500) {
RPM2 = map(RPM, 0, 7500, 0, 5840); 
}
}
if(GRPMOND == false) {
RPM2 = RPM;
}
if (inSweepWindow) {
RPM2 = 6000; // dial's printed max, not the 7500 software ceiling used for normal driving
}

idleRPM += 10;
RPM2 += 4;
if(gas == 0 && Speed > 5 && RPM > idleRPM ) {
efficient = 0xF6;
} else {
efficient = 0xF0;
}

// RPM tach message (0xF3) - reverted (2026-08-29) back to this original 16-bit
// encoding. The reference-matched single-byte restructure (tried 2026-08-28) did fix
// the ~2000rpm cap but wasn't a satisfying result live in-game (still "not quite
// right" per user, wanted the old feel back) - reverting rather than continuing to
// patch blind. If the >2000rpm range issue is revisited, don't just reapply the
// same single-byte rewrite - it traded away feel for range and that trade wasn't
// wanted. Needs a different approach (possibly matching the reference project's structure AND
// finding real hardware confirmation of the actual field width/resolution this
// specific cluster expects, rather than assuming the reference project's exact numbers apply here).
// TEST (2026-09-23): rpm_frame[3] changed from fixed 0xC0 to 0x00, to check
// whether this byte controls the small instant-consumption (0-20 L/100km) needle
// the user noticed below the tach. Neither this project's nor the reference project's
// reference documents this byte as controlling anything - both have it as a
// constant filler value (the reference project uses 0xC0 here too, so this project already
// matched the reference project before this test). If the needle doesn't move at all after this
// change, revert to 0xC0 and try changing rpm_frame[5] (currently fixed 0xC4)
// instead as the next candidate.
if (rpmMode == 2) {
  // the reference project layout: byte1 = 0x60 | alive counter, byte2 = rpm / ~160 (coarse), byte3 0xC0
  uint16_t v = (uint16_t)(RPM2 * 1.557);
  uint8_t f3[8] = { 0, (uint8_t)(0x60 | counter4Bit), (uint8_t)(v >> 8), 0xC0, (uint8_t)efficient, 0x00, 0xFF, 0xFF };
  applyOv(0xF3, f3, 8);
  f3[0] = crc8Calculator.get_crc8(f3 + 1, 7, (uint8_t)crcSeedF3);
  CAN.sendMsgBuf(0xf3, 0, 8, f3);
} else if (rpmMode == 1) {
  uint8_t f3[8] = { 0, (uint8_t)(int(RPM2 * 1.557) & 0xff), (uint8_t)(int(RPM2 * 1.557) >> 8), 0x00, (uint8_t)efficient, 0xC4, 0xFF, 0xFF };
  applyOv(0xF3, f3, 8);
  f3[0] = crc8Calculator.get_crc8(f3 + 1, 7, (uint8_t)crcSeedF3);
  CAN.sendMsgBuf(0xf3, 0, 8, f3);
} else {
uint8_t rpm_frame[8] = {0xF3, 0x4A, 0x06, 0x00, efficient, 0xC4, 0xFF, 0xFF   };

    rpm_frame[1] = (int(RPM2 * 1.557) & 0xff);
    rpm_frame[2] = (int(RPM2 * 1.557) >> 8);
    applyOv(0xF3, rpm_frame, 8);
    uint8_t rpm_crc = crc8(rpm_frame, sizeof(rpm_frame));
    rpm_frame[0] = crc;
  CAN.sendMsgBuf(0xf3, 0, 8, rpm_frame);


RPM2 -= 4;

    rpm_frame[1] = (int(RPM2 * 1.557) & 0xff);
    rpm_frame[2] = (int(RPM2 * 1.557) >> 8);
    applyOv(0xF3, rpm_frame, 8);
    crc = crc8(rpm_frame, sizeof(rpm_frame));
    rpm_frame[0] = crc;
  CAN.sendMsgBuf(0xf3, 0, 8, rpm_frame);
}




//MPG/range bar - byte0 must be a clean incrementing counter (matches the reference project's
//sendDistanceTravelled()) and the CRC poly is 0xC6, not 0xFF. The previous
//`throthel | counter4Bit` byte0 and wrong 0xFF poly produced a checksum that only
//coincidentally matched what the cluster expects, causing the range/consumption bar
//to update inconsistently instead of tracking real fuel usage.
    unsigned char mpgWithoutCRC[] = { (uint8_t)(fuelK ? (long)fuelAccum & 0xFF : count), 0xFF, 0x64, 0x64, 0x64, 0x01, 0xF1 };
    if (fuelByte > 0 && fuelByte < 6) {
      uint16_t fa = (uint16_t)fuelAccum;
      mpgWithoutCRC[0] = count;
      mpgWithoutCRC[fuelByte] = fa & 0xFF;
      mpgWithoutCRC[fuelByte + 1] = fa >> 8;
    }
    applyOv(0x2C4, mpgWithoutCRC, 7);
    unsigned char mpgWithCRC[] = { crc8Calculator.get_crc8(mpgWithoutCRC, 7, 0xC6), mpgWithoutCRC[0], mpgWithoutCRC[1], mpgWithoutCRC[2], mpgWithoutCRC[3], mpgWithoutCRC[4], mpgWithoutCRC[5], mpgWithoutCRC[6] };
    if (c4hdr) {
      uint8_t f[8] = { 0, (uint8_t)(c4hdr | counter4Bit), 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      applyOv(0x2C5, f, 8);   // "o 2c5 idx val" addresses this experimental frame by absolute byte index
      f[0] = crc8Calculator.get_crc8(f + 1, 7, (uint8_t)c4seed);
      CAN.sendMsgBuf(0x2C4, 0, 8, f);
    } else
    CAN.sendMsgBuf(0x2C4, 0, 8, mpgWithCRC);
	  

//distance travelld counter
// Gated to a real 100ms interval (matching the reference project's dashboardUpdateTime100),
// unlike the rest of this Loop() which runs unthrottled on every call. Without this
// gate the odometer/distance value inflates far faster than real elapsed time, since
// Loop() itself runs many times faster than 10Hz — this was observed live as the
// cluster's total km jumping ~1000km after only a few minutes of driving.
static unsigned long lastDistanceUpdateMs = 0;
if (millis() - lastDistanceUpdateMs >= 100) {
    lastDistanceUpdateMs = millis();
    unsigned char mpg2WithoutCRC[] = { 0xF0|counter4Bit, lo8(distanceTravelledCounter), hi8(distanceTravelledCounter), 0xF2 };
    unsigned char mpg2WithCRC[] = { crc8Calculator.get_crc8(mpg2WithoutCRC, 4, 0xde), mpg2WithoutCRC[0], mpg2WithoutCRC[1], mpg2WithoutCRC[2], mpg2WithoutCRC[3], mpg2WithoutCRC[4] };
    CAN.sendMsgBuf(0x2BB, 0, 5, mpg2WithCRC);
    // REVERTED (2026-09-22) after the *58 experiment (20x) broke real
    // functionality: above ~35km/h the cluster stopped counting distance
    // altogether (and consequently the range/consumption display stopped
    // decreasing while driving). Root cause is believed to be either 16-bit
    // (`int`) wraparound of this counter happening too fast at highway speed
    // with such a large multiplier, or the cluster's own plausibility filter on
    // this pulse-style CAN value rejecting per-tick deltas that large as
    // physically impossible - either way, this is a real regression, not just a
    // cosmetic accuracy issue. Before this, *29 (10x) was tried too: range only
    // went 140->174km (nowhere near the ~10x a linear ratio would predict), and
    // average consumption was separately observed to read ~15 while driving
    // gently but jump to a sticky 29.5 after hard acceleration that never came
    // back down even with subsequent gentle driving - that "spikes and never
    // decays" behavior looks like a latch/fault-style stuck state (same class as
    // this cluster's other "needs a real 12V cycle to clear" quirks documented
    // elsewhere in CLAUDE.md), not a pure distance/fuel ratio problem. A
    // power-cycle+gentle-driving-only test to isolate that was proposed but not
    // done before escalating to *58 instead. **Don't retry a distance-multiplier
    // hack on this line again** - both *29 and *58 were tried, *58 actively broke
    // odometer counting above 35km/h, and *29's improvement was marginal at best
    // and possibly illusory (see the sticky-29.5 finding). Also note: the real
    // permanent odometer (441,614 km baseline) was inflated by both the *29 and
    // *58 test drives - that inflation is NOT undone by reverting this line,
    // there's no known way to reset it.
    // REVERTED TO SAFE BASELINE (2026-09-23, end of session). The whole *29/*58/
    // *100/*150 multiplier saga above was chasing the wrong cause: the range
    // display problems that motivated it turned out to trace back to a stale/
    // unsaved `customprotocol` in SimHub (see the "RESOLUTION" entry elsewhere in
    // CLAUDE.md), not insufficient distance data. Confirmed this line was still
    // at *58 well after that was fixed, and it was causing a NEW, real symptom:
    // range showing "----" (undisplayable) at highway speed with a well-over-
    // half-full tank - consistent with the already-documented "odometer stops
    // counting above ~35km/h at *58" bug finally manifesting during normal
    // driving. No remaining reason to keep any multiplier above the original
    // *2.9 - reverted. **Don't re-attempt this multiplier hack without
    // genuinely new reasoning** - it was based on a misdiagnosis, and every
    // value tried above baseline (*29, *58, *100, *150) either did nothing
    // reliable or actively broke odometer counting at real driving speeds.
    distanceTravelledCounter += Speed*2.9*distMul;
    if (fuelRaw16 >= 0 && Speed > 0) fuelRaw16 += fuelDrain / 100.0;
    fuelAccum += (float)Speed * throthel * fuelK / 1000000.0 * distMul;
    if (fuelAccum >= 65536.0) fuelAccum -= 65536.0;
}
    
//SteeringWheel error clear
    unsigned char steeringColumnWithoutCRC[] = { 0xF0|counter4Bit, 0xFE, 0xFF, 0x14 };
    unsigned char steeringColumnWithCRC[] = { crc8Calculator.get_crc8(steeringColumnWithoutCRC, 4, 0x9E), steeringColumnWithoutCRC[0], steeringColumnWithoutCRC[1], steeringColumnWithoutCRC[2], steeringColumnWithoutCRC[3] };
    CAN.sendMsgBuf(0x2A7, 0, 5, steeringColumnWithCRC);
    
    
//Fuel nedel
  boolean isCarMini = false;
  // Head-on table (2026-09-28): 4 = full, 8 = 3/4, 11 = 60 %, 18 = 1/2, 27 = 1/4, 37 = empty.
  // The ~+20 % fuel correction the cluster learned that evening was gone the next day
  // (game full showed ~76 % = raw 8 -> 3/4 again), so this original table is back.
  uint8_t inFuelRange[] = {0, 25, 50, 60, 75, 100};
  uint8_t outFuelRange[] = {37, 27, 18, 11, 8, 4};
  uint8_t fuelQuantityLiters = multiMap<uint8_t>(fuelpercentage, inFuelRange, outFuelRange, 6);
  // (2026-09-23) No forced-empty test window anymore - see the removal note near
  // the top of Loop() for why. This is always just the real computed value now.
  // The gauge is still known to be heavily damped/slow inside the cluster's own
  // hardware (see the older fuel-sweep-speed entries in CLAUDE.md) - that part is
  // untouched and still applies.
  if (fuelRaw16 >= 0) {
    uint16_t fr = (uint16_t)fuelRaw16;
    unsigned char fuelFine[] = { (uint8_t)(fr & 0xFF), (uint8_t)(fr >> 8), (uint8_t)(fr & 0xFF), (uint8_t)(fr >> 8), 0x00 };
    CAN.sendMsgBuf(0x349, 0, 5, fuelFine);
  } else {
  unsigned char fuelWithoutCRC[] = { (isCarMini ? 0 : hi8(fuelQuantityLiters)), (isCarMini ? 0 : lo8(fuelQuantityLiters)), hi8(fuelQuantityLiters), lo8(fuelQuantityLiters), 0x00 };
  CAN.sendMsgBuf(0x349, 0, 5, fuelWithoutCRC);
  }
  
  
//gear display(A)
  gmax += 1;
if (gmax == 0x09) {
gmax = 0;
}
    
selectedGear = 0x00;
manual = counter4Bit;

bool gearMatched = false;

if (gear == "1") {
manual = 0x10;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "2") {
manual = 0x20;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "3") {
manual = 0x30;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "4") {
manual = 0x40;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "5") {
manual = 0x50;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "6") {
manual = 0x60;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "7") {
manual = 0x70;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "8") {
manual = 0x80;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
} else if(gear == "9") {
manual = 0x90;
runsg = manual;
manual += gmax;
runs = 0;
selectedGear = 0x81;
gearMatched = true;
}

if(gear == "N" or gear == "0" or gear == " ") {
runsb = runsg;
runsb += gmax;
manual = runsb;


runs += 1;
selectedGear = 0x60;
gearMatched = true;
}

if(runs > 20) {
manual = counter4Bit;
}

if ( gear == "R") {
selectedGear = 0x40;
manual = counter4Bit;
gearMatched = true;
}

// Automatic drive: none of the digit/N/R branches matched (this is what ETS2
// reports while in automatic mode), so show D instead of leaving it blank.
if (!gearMatched) {
selectedGear = 0x80;
manual = counter4Bit;
}
    unsigned char transmissionWithoutCRC[] = { manual, selectedGear, 0xFC, 0xFF }; //0x20= P, 0x40= R, 0x60= N, 0x80= D, 0x81= DS
    unsigned char transmissionWithCRC[] = { crc8Calculator.get_crc8(transmissionWithoutCRC, 4, 0xD6), transmissionWithoutCRC[0], transmissionWithoutCRC[1], transmissionWithoutCRC[2], transmissionWithoutCRC[3] };
    CAN.sendMsgBuf(0x3FD, 0, 5, transmissionWithCRC);
    
    
//check engine
if(checkengwhenoff == false) {
EngineIgnitionOn = 1;
}
  if(oilpress == 1 or EngineIgnitionOn == 0) {
      uint8_t message2[] = { 0x40, 34, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(message2);
} else {
	      uint8_t message2[] = { 0x40, 34, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(message2);
}


// Kept hardcoded off deliberately: code 71 was confirmed (2026-08-28) to also
// toggle the parking-brake lamp on its own, unlike code 77 below which is clean.
// EXPLAINED (2026-08-29): The reference project labels code 71 as
// "Park brake error (red)", NOT seatbelt - this project's original variable name
// ("seat_belt_indecator") was simply wrong. See CLAUDE.md before ever driving this
// one from the switch.
uint8_t seat_belt_indecator[] = { 0x40, 71, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
send5c0(seat_belt_indecator);



//dscwarning
if(dscWarning == 1) {
      uint8_t dscWarning[] = { 0x40, 215, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(dscWarning);
} else {
	      uint8_t dscWarning1[] = { 0x40, 215, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(dscWarning1);
}


// Brake air pressure critical warning (2026-09-23). Uses 0x5c0 fault code 24
// ("park brake error - yellow" per the reference project), which was
// completely unused until now - distinct from code 71 ("park brake error -
// red", permanently disabled, see the seatbelt/parking-brake entry in
// CLAUDE.md) and from the real handbrake-engaged light on 0x34F. User wanted
// the yellow lamp specifically because they'd seen it flash yellow on
// startup, and wanted to reuse it for "brake air pressure low".
// `acc_lightstage` (customprotocol field 21, previously always 0/dead - see
// grep confirming it was otherwise unreferenced) now carries raw ETS2 brake
// air pressure via
// DataCorePlugin.GameRawData.TruckValues.CurrentValues.MotorValues.BrakeValues.AirPressure.
// CONFIRMED live (2026-09-23) - user already uses this exact property as a
// level indicator on their separate button box, reading ~117 at the time this
// was confirmed, so the earlier "never verified live" caution from the field-3
// pressure-needle attempt no longer applies to this property. User specified
// 80 as the real desired warning threshold (not a guess).
if (acc_lightstage > 0 && acc_lightstage < 80) {
      uint8_t brakeAirWarning[] = { 0x40, 24, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(brakeAirWarning);
} else {
      uint8_t brakeAirWarning1[] = { 0x40, 24, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(brakeAirWarning1);
}


//send drivingmode
unsigned char modeWithoutCRC[] = { 0xF0|counter4Bit, 0, 0, drivemod, 0x11, 0xC0 };
unsigned char modeWithCRC[] = { crc8Calculator.get_crc8(modeWithoutCRC, 6, 0x4a), modeWithoutCRC[0], modeWithoutCRC[1], modeWithoutCRC[2], modeWithoutCRC[3], modeWithoutCRC[4], modeWithoutCRC[5] };
CAN.sendMsgBuf(0x3A7, 0, 7, modeWithCRC);


//clear ABS error
unsigned char abs1WithoutCRC[] = { 0xF0|counter4Bit, 0xFE, 0xFF, 0x14 };
unsigned char abs1WithCRC[] = { crc8Calculator.get_crc8(abs1WithoutCRC, 4, 0xD8), abs1WithoutCRC[0], abs1WithoutCRC[1], abs1WithoutCRC[2], abs1WithoutCRC[3] };
CAN.sendMsgBuf(0x36E, 0, 5, abs1WithCRC);


//alivecounterSaftey error clear
unsigned char aliveCounterSafetyWithoutCRC[] = { counter4Bit, 0xFF };
CAN.sendMsgBuf(0xD7, 0, 2, aliveCounterSafetyWithoutCRC);


//RestrainAribag error clear
unsigned char restraintWithoutCRC[] = { 0x40 | counter4Bit, 0x40, 0x55, 0xFD, 0xFF, 0xFF, 0xFF };
unsigned char restraintWithCRC[] = { crc8Calculator.get_crc8(restraintWithoutCRC, 7, 0xFF), restraintWithoutCRC[0], restraintWithoutCRC[1], restraintWithoutCRC[2], restraintWithoutCRC[3], restraintWithoutCRC[4], restraintWithoutCRC[5], restraintWithoutCRC[6] };
CAN.sendMsgBuf(0x19B, 0, 8, restraintWithCRC);        


//RestrainSeatbelt error clear and lamp off
unsigned char restraint2WithoutCRC[] = { 0xE0 | counter4Bit, 0xF1, 0xF0, 0xF2, 0xF2, 0xFE };
unsigned char restraint2WithCRC[] = { crc8Calculator.get_crc8(restraint2WithoutCRC, 6, 0x28), restraint2WithoutCRC[0], restraint2WithoutCRC[1], restraint2WithoutCRC[2], restraint2WithoutCRC[3], restraint2WithoutCRC[4], restraint2WithoutCRC[5] };
CAN.sendMsgBuf(0x297, 0, 7, restraint2WithCRC); // clears error
// Real seatbelt lamp, driven by the buckle switch (button 5, pin 7). Confirmed
// (2026-08-28) this code is clean - does NOT affect the parking-brake lamp, unlike
// code 71 above.
uint8_t Seatbeltlamp[] = {  0x40, 77, 0x00, (uint8_t)(seatbeltBuckled ? 0x28 : 0x29), 0xFF, 0xFF, 0xFF, 0xFF};
send5c0(Seatbeltlamp); //driven by real buckle switch, see above


//SOS error clear
sos += 1;
if (sos == 0xff) {
sos = 0;
}
uint8_t SOS[] = {  sos, sos, sos, sos, sos, sos, sos, sos };
CAN.sendMsgBuf(0x2C3, 0, 8, SOS);
    
    
//Tire Presure monitor faliure error clear
tpms += 1;
if(tpms == 0xff) {
tpms = 0;
}
uint8_t TPMS[] = {  tpms, tpms, tpms, tpms, tpms, tpms, tpms, tpms };
CAN.sendMsgBuf(0x368, 0, 8, TPMS);


//enable or disable fogg highbeam and low beam indecator
// highbeam is checked first so that light==1 && highbeam==1 (low beam still
// reported on while high beam is engaged) is not left unhandled and shown as "off"
if (lightsOverrideOn) {
    applyOv(0x21A, lightsOverride, 3);
    CAN.sendMsgBuf(0x21A, 0, 3, lightsOverride);
} else if((highbeam == 1) && (fogg == 1)) {
    uint8_t data3[] = {  0x67, 0x00, 0xf7};
     CAN.sendMsgBuf(0x21A, 0, 3, data3);

} else if((highbeam == 1) && (fogg == 0)) {
    uint8_t data7[] = {  0x07, 0x00, 0xf7};
     CAN.sendMsgBuf(0x21A, 0, 3, data7);

} else if((light == 0) && (fogg == 1)) {
    uint8_t data1[] = {  0x60, 0x00, 0xf7};
    CAN.sendMsgBuf(0x21A, 0, 3, data1);

} else if((light == 1) && (fogg == 1)) {
    uint8_t data4[] = {  0x65, 0x00, 0xf7};
     CAN.sendMsgBuf(0x21A, 0, 3, data4);

} else if((light == 1) && (fogg == 0)) {
    // UNRESOLVED (2026-09-23), reverted to original baseline for now. Tried 8
    // combinations live: 0x04/0x00, 0x05/0x00 (this original value), 0x06/0x00,
    // 0x08/0x00, 0x0C/0x00, 0x10/0x00, 0x05/0xC0 (the reference project's byte1), 0x05/0x12
    // (a real captured E87/E90 frame from a hobbyist CAN-sniffing project) -
    // NONE showed a distinct low-beam icon; the cluster shows a green
    // "parking lights" icon instead whenever byte0's bit2 is set, and shows
    // that PLUS the correct blue high-beam icon when high beam is genuinely on
    // (0x07/0x00, unchanged, still confirmed working). See CLAUDE.md for the
    // full trail before trying more values - reverted to this original 0x05
    // baseline (no evidence it's more/less correct than the others tried, but
    // it matches what an earlier session claimed was "confirmed working" at
    // some point). Needs real CAN sniffing to solve properly.
    uint8_t data5[] = {  0x05, 0x00, 0xf7};
     CAN.sendMsgBuf(0x21A, 0, 3, data5);

   } else {
	    uint8_t data8[] = {  0x00, 0x00, 0xf7};
     CAN.sendMsgBuf(0x21A, 0, 3, data8);
}

//backlight brightness: reverted (2026-09-19) back to headlights-only, undoing the
//2026-08-28 change that tied it to ignition instead. User now wants the panel dark
//whenever headlights/parking lights are off, even with ignition on.
//Must send an explicit 0 when off, not just skip sending - confirmed live
//(2026-09-19) that the cluster holds the last brightness it received and never
//dims on its own if the message simply stops, so the off-branch is not optional.
//`showLights` (customprotocol field 53) repurposed here to carry ETS2's parking-
//lights state (DataCorePlugin...LightsValues.Parking) - the field/variable was
//previously dead (parsed, never used) since this sketch's original template had
//no distinct parking-lights concept.
if (light == 1 || showLights == 1) {
    uint8_t mappedBrightness = map(backlight, 0, 100, 0, 253);
    unsigned char backlightBrightnessWithoutCRC[] = { mappedBrightness, 0xFF };
    applyOv(0x202, backlightBrightnessWithoutCRC, 2);
    CAN.sendMsgBuf(0x202, 0, 2, backlightBrightnessWithoutCRC);
} else {
    unsigned char backlightOffWithoutCRC[] = { 0, 0xFF };
    applyOv(0x202, backlightOffWithoutCRC, 2);
    CAN.sendMsgBuf(0x202, 0, 2, backlightOffWithoutCRC);
}


//set units
//76 km/l km
//66 mpg km
//89 l/100 km
//206 km/l mi
//147 mpg mi
// l/100  mi
// 18 c°
// 34 f°
if(setcorf == 1) {
byte2 = 18;
} else {
byte2 = 34;
}
if(setmpgl100orkml == 1 && setkmormiles == 1) {
byte3 = 89;
} else if(setmpgl100orkml == 2 && setkmormiles == 1) {
byte3 =	66;
} else if(setmpgl100orkml == 3 && setkmormiles == 1) {
byte3 = 76;
} else if(setmpgl100orkml == 1 && setkmormiles != 1) {
byte3 = 145;
} else if(setmpgl100orkml == 2 && setkmormiles != 1) {
byte3 = 147;
} else if(setmpgl100orkml == 3 && setkmormiles != 1) {
byte3 = 206;
}
  uint8_t cel[] = {  setlanguage, byte2, byte3, 0x00, 0x00, 0x00, 0x00, 0x00};
    CAN.sendMsgBuf(0x291, 0, 8, cel);

//code part where u need the partch

if(EDITEDSELUA == true) {
// cruisecontrol
  if(cruise == 1) {
cv = 0x96;
} else {
cv = 0x00;
}
  unsigned char cruiseWithoutCRC[] = { 0xF0|counter4Bit, 0x00, 0xE0, 0xE1, cv, 0x14, 0x00  };
  applyOv(0x289, cruiseWithoutCRC, 7);
  unsigned char cruiseWithCRC[] = { crc8Calculator.get_crc8(cruiseWithoutCRC, 7, 0x82), cruiseWithoutCRC[0], cruiseWithoutCRC[1], cruiseWithoutCRC[2], cruiseWithoutCRC[3], cruiseWithoutCRC[4], cruiseWithoutCRC[5], cruiseWithoutCRC[6] };
 CAN.sendMsgBuf(0x289, 0, 8, cruiseWithCRC);

// EXPERIMENT (2026-09-23): cruise control SET SPEED, not on/off (that's 0x289
// above - this is separate). CAN ID and byte layout taken from a community BMW
// E90 DBC file (a public BMW E90 DBC file), NOT from the reference project's
// reference or this project's own testing - E90 is an OLDER BMW generation than
// this cluster's F30, and BMW's own docs note the KOMBI/Gateway CAN protocol
// differs across generations ("speak the same language but not the same
// accent"), so this ID/layout is UNCONFIRMED for this specific F30 cluster -
// treat as a real experiment, not a known-good value. DBC signal:
// "CruiseControlSetpoint", byte 1, scale 1 offset -2 (i.e. raw = km/h + 2). No
// checksum/counter byte is documented for this message in the DBC (unlike most
// other messages in this file) - sent as-is, byte0 and trailing bytes are
// guesses (0x00/0xFF filler) since the DBC only documents the one signal.
// `oil_warn` (customprotocol field 11, previously dead) carries
// DashboardValues.CruiseControlSpeed + 2 - that ETS2 property itself is ALSO
// unconfirmed (guessed by analogy to the existing DashboardValues.CruiseControl
// on/off property) - check Available Properties for "Cruise" while driving with
// cruise control set to confirm the real property name before trusting this.
// If the cluster shows nothing or a wrong number, don't assume the code logic
// is broken - re-verify the ETS2 property name first (same failure mode as
// every other guessed property in this file - silently reads 0/wrong, no error).
//
// UPDATE (2026-09-23): raw/no-checksum version above confirmed NOT working live
// (SimHub-side data independently confirmed correct via Ncalc Tester - see
// CLAUDE.md). Cross-checked several of THIS cluster's own confirmed-working CAN
// IDs against the E90 DBC: 6 of 23 matched exactly (0x349, 0x34F, 0x202, 0x21A,
// 0xD7, 0x1F6), showing real CAN-architecture continuity between E90 and F30 for
// body/dash messages - so the earlier "wrong generation" theory is weaker than
// first thought. Every other working message on this cluster uses this
// project's own CRC-then-alive-counter framing (byte0=CRC8 over the rest, next
// byte=`0xF0|counter4Bit`) via `crc8Calculator.get_crc8()` - restructured to
// match that convention instead of the DBC's raw/uncheck-summed layout, betting
// that this cluster's own established framing is more trustworthy than an
// incomplete reverse-engineered DBC. **The CRC seed/poly (0x82 here) is an
// UNVERIFIED GUESS** - reused from this project's own 0x289 cruise-on/off
// message as an arbitrary starting point, no evidence it's correct for 0x193.
// Checked for an arithmetic relationship between other messages' CAN IDs and
// their known-correct seeds (e.g. XOR/subtraction of the ID's low byte) -
// found none (0x2A7 and 0x3A7 share the same low byte 0xA7 but use different
// seeds 0x9E/0x4a), so there's no shortcut to derive the right seed - this is a
// blind, low-odds (~1/256) guess, expect to try several values before
// concluding whether a checksum was even the real problem.
// DISABLED (2026-09-23): suspected of interfering with the (previously
// confirmed-working) brake-air-pressure lamp on 0x5c0 code 24 - user reported
// that lamp stopped working right after these two experimental E90-DBC-based
// messages (0x193, 0x2F8) were added. Since 0x193 is a GUESSED ID for this F30
// generation, it's entirely possible it actually means something else on this
// specific cluster/bus and its guessed payload is confusing some other real
// function, not just being ignored - unlike a "silently ignored" bad guess,
// this would be an active side effect. Disabled to test whether removing it
// restores the air-pressure lamp. Don't re-enable without new evidence this ID
// is actually safe on this generation.
/*
uint8_t cruiseSpeedWithoutCRC[] = { (uint8_t)(0xF0|counter4Bit), (uint8_t)oil_warn, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t cruiseSpeedWithCRC[] = { crc8Calculator.get_crc8(cruiseSpeedWithoutCRC, 7, 0x82), cruiseSpeedWithoutCRC[0], cruiseSpeedWithoutCRC[1], cruiseSpeedWithoutCRC[2], cruiseSpeedWithoutCRC[3], cruiseSpeedWithoutCRC[4], cruiseSpeedWithoutCRC[5], cruiseSpeedWithoutCRC[6] };
CAN.sendMsgBuf(0x193, 0, 8, cruiseSpeedWithCRC);
*/

// EXPERIMENT (2026-09-23): clock/date on the MID screen. Same community BMW E90
// DBC source and same generation-gap caveat as the cruise-set-speed block above
// - CAN ID 0x2F8, message "DATE_2F8", byte layout: byte0=hour, byte1=minute,
// byte2=second, byte3=day, byte4=(month<<4)|0x0F (top nibble only, low nibble
// documented as always 0xF), bytes5-6=year as a 16-bit little-endian value,
// byte7 undocumented/filler. No checksum/counter documented either - sent raw.
// `DataCorePlugin.CurrentDateTime` is real-world PC clock time (not simulated
// in-game time), extracted into 6 separate customprotocol fields via SimHub's
// format() function with .NET-style date format specifiers ('HH','mm','ss',
// 'dd','MM','yyyy') - this specific use of format() on a DateTime-typed
// property (as opposed to the numeric formatting format() is used for
// elsewhere in this file, e.g. field 1's speed) is UNCONFIRMED to work the way
// assumed here; verify each field's actual output in SimHub's Ncalc Tester
// before assuming the byte values below are correct if the clock shows garbage.
// Field mapping: H=hour(35), absWarning=minute(16), dscSwitch=second(18),
// pcars_mcarflags=day(20), acc_flashlight=month(22), checkEngine_ETS=year(6).
//
// UPDATE (2026-09-23): same checksum-guess restructure as the cruise-speed
// message above, same reasoning (SimHub data confirmed correct, 6/23 CAN IDs
// match E90 exactly so generation gap alone is a weaker explanation, this
// cluster's own messages all use CRC+counter framing) and same caveat (seed
// 0x82 is an unverified blind guess, ~1/256 odds, no derivable pattern found).
// Reused 0x82 here too (arbitrary - as good a first guess as any other unused
// value). Dropped the alive-counter byte for THIS message specifically (unlike
// cruise speed above) to keep all 7 date/time values intact within the 7 bytes
// available after the CRC byte - if this needs the counter convention instead,
// year would need to drop to one byte or a different field would need to be
// sacrificed.
// DISABLED (2026-09-23): same reason as the 0x193 cruise-speed block above -
// suspected of interfering with the brake-air-pressure lamp, disabled to test
// whether removing it restores that lamp. Don't re-enable without new evidence
// 0x2F8 is safe on this generation.
/*
uint16_t clockYear = (uint16_t)checkEngine_ETS;
uint8_t clockWithoutCRC[] = { (uint8_t)H, (uint8_t)absWarning, (uint8_t)dscSwitch, (uint8_t)pcars_mcarflags, (uint8_t)((acc_flashlight << 4) | 0x0F), lo8(clockYear), hi8(clockYear) };
uint8_t clockWithCRC[] = { crc8Calculator.get_crc8(clockWithoutCRC, 7, 0x82), clockWithoutCRC[0], clockWithoutCRC[1], clockWithoutCRC[2], clockWithoutCRC[3], clockWithoutCRC[4], clockWithoutCRC[5], clockWithoutCRC[6] };
CAN.sendMsgBuf(0x2F8, 0, 8, clockWithCRC);
*/

 
//tire flat
if (tfl == 1) {
  uint8_t tfld[] = { 0x40, 0x8B, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(tfld);
} else {
  uint8_t tfld[] = { 0x40, 0x8B, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(tfld);
 }

if (tfr == 1) {
  uint8_t tfrd[] = { 0x40, 0x8F, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(tfrd);
} else {
  uint8_t tfrd[] = { 0x40, 0x8F, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(tfrd);
 }
 
 if (trl == 1) {
  uint8_t trld[] = { 0x40, 0x8D, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(trld);
} else {
  uint8_t trld[] = { 0x40, 0x8D, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(trld);
 }
 
 if (trr == 1) {
  uint8_t trrd[] = { 0x40, 0x8C, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(trrd);
} else {
  uint8_t trrd[] = { 0x40, 0x8C, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(trrd);
 }


//doors, trunk, frunk open or closed
if (dfl == 1) {
  uint8_t dfld[] = { 0x40, 0xF, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(dfld);
} else {
  uint8_t dfld[] = { 0x40, 0xF, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(dfld);
 }

if (dfr == 1) {
  uint8_t dfrd[] = { 0x40, 0xE, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(dfrd);
} else {
  uint8_t dfrd[] = { 0x40, 0xE, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(dfrd);
 }
 
 if (drl == 1) {
  uint8_t drld[] = { 0x40, 0x10, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(drld);
} else {
  uint8_t drld[] = { 0x40, 0x10, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(drld);
 }
 
 if (drr == 1) {
  uint8_t drrd[] = { 0x40, 0x11, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(drrd);
} else {
  uint8_t drrd[] = { 0x40, 0x11, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(drrd);
 }

 if (hood == 1) {
  uint8_t hoodd[] = { 0x40, 0x12, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(hoodd);
} else {
  uint8_t hoodd[] = { 0x40, 0x12, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(hoodd);
 }
 
 if (trunk == 1) {
  uint8_t trunkd[] = { 0x40, 0x13, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(trunkd);
} else {
  uint8_t trunkd[] = { 0x40, 0x13, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      send5c0(trunkd);
 }
 // other found ids but dont used jet
 /*
    uint8_t dataa[] = {  A, A, A, A, A, A, A, A}; 2 = on 1 = off
    sendCANMessage(0x36A, dataa, 8); // Automatic high beam light


  uint8_t dataa[] = {  1AA, 1AA, 1AA, 1AA, 1AA, 1AA, 1AA, 1AA}; 1AA = on E6= off
    sendCANMessage(0x30B, dataa, 8); // Auto start stop /
    */
}
}
};
#endif // __SHCUSTOMPROTOCOL_H__
