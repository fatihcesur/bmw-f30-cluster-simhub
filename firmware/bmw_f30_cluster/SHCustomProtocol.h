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
extern bool nightThemeWithLights;
extern bool lightsIconViaFog;
extern bool backlightAlwaysOn;
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
int throthel = 0;       // field 34: fuel left in a 52 L car tank, centilitres (FuelPercent x 52)
float fuelUsed = 0;     // 0x2C4 byte0 fuel counter, see the 0x2BB block
bool gotSimHubData = false;  // stay off the CAN bus until SimHub has sent real values
bool refuelIgnition = false;  // show "ignition on" in 0x12F while refuelling (see Loop)
const unsigned long REFUEL_IGN_MS = 120000UL;
bool refuelFull = false;      // send 100 % while a refuel is running (REFUEL_MODE bit1)
unsigned long refuelSpeedHoldUntil = 0;   // 0x1A1 says 0 km/h until then (after a refuel wake)
const unsigned long REFUEL_HOLD_MS = 5000;
#ifndef REFUEL_MODE
#define REFUEL_MODE 1   // bench 2026-10-04: bit1 (send 100 %) froze the needle, bit0 + speed hold works
#endif
int H = 0;
int braketemp = 0;
String Game = "ETS2";
int fuelpercentage = 0;
int oilpress = 0;
int checkEngine_ETS = 0; // repurposed: clock year for the 0x39E set-time frame
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
int tempomats = 0;   // field 45: ETS2 warning bits, see the check-control block after the tire/door codes
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

class SHCustomProtocol {

public:

void Setup() { 
	    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        Serial.println(F("MCP2515 Initialized Successfully!"));
        CAN.setMode(MCP_NORMAL);
    } else {
        Serial.println(F("Error Initializing MCP2515..."));
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
      gotSimHubData = true;
}



void Loop() {
// Until the first SimHub packet the variables are defaults (fuel 0 %, ...);
// sending those made the fuel needle drift towards empty after every SimHub
// restart. Staying silent lets the cluster sleep like a parked car instead.
if (!gotSimHubData) return;

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
// Fuel gauge "wake" (2026-09-28, measured on the webcam): the cluster filters the
// fuel level heavily (~2 deg/min while driving, ~30 s full sweep when stopped),
// but after >= 6 s of total CAN silence it wakes up and the needle jumps to the
// real level in < 5 s. So on ignition-on, and when fuel goes up while stopped
// (refuelling), send nothing for WAKE_MUTE_MS. Never while moving: silence at
// speed latches the l/100km needle at 20. The welcome sweep runs after the mute.
// ETS2 fills the tank gradually. A wake while the level is still rising does NOT
// move the needle (bench 2026-10-04, captures/refuel2: only the wake after the level
// stopped changing jumped it), so wait until the fuel has been steady for
// FUEL_STEADY_MS before muting.
const unsigned long WAKE_MUTE_MS = 7000;
const unsigned long FUEL_STEADY_MS = 2500;
static unsigned long wakeMuteUntil = 0;
static bool wakeMuting = false;
static int fuelAtStop = -1;
static int lastFuel = -1;
static unsigned long fuelSteadySince = 0;
// While ETS2 loads (truck switch, quick job) SimHub has no data and the formula sends fuel 0
// for ~15 s (game2/game.csv 18:44:37). Sudden "empty" taught the cluster a fuel correction:
// afterwards full showed 1/2 and half showed empty (bench switch1). A jump straight to 0
// from more than 2 % is never real fuel use, so keep the last good level instead.
static int goodFuel = -1;
if (fuelpercentage == 0 && goodFuel > 2) fuelpercentage = goodFuel;
else goodFuel = fuelpercentage;
static unsigned long fuelRiseAt = 0;
static unsigned long lastRiseAt = 0;       // last rise while stopped, any ignition state
static bool refuelWoke = false;
static unsigned long refuelStartAt = 0;    // first rise of this stop
if (Speed > 2) refuelStartAt = 0;
if (fuelpercentage != lastFuel) {
  if (lastFuel >= 0 && fuelpercentage > lastFuel && Speed <= 2) {
    fuelRiseAt = lastRiseAt = millis();
    if (refuelStartAt == 0) refuelStartAt = millis();
  }
  lastFuel = fuelpercentage;
  fuelSteadySince = millis();
}
// In-game (2026-10-04, SimHub API): ETS2 keeps the ignition ON while refuelling (engine off)
// and fills in ~10 s; the player drives off right after. The needle only gets ~75 % of the
// way at standstill speed (~30 s full sweep, slowing near the target). REFUEL_MODE:
// bit0 = wake (7 s CAN silence) as soon as the fill starts, bit1 = send 100 % while filling.
bool refuelFilling = lastRiseAt != 0 && Speed <= 2 && millis() - lastRiseAt < 1500;
if (Speed > 2) refuelWoke = false;
// With the ignition off the cluster must first be shown "ignition on" (refuelIgnition) for
// a moment: put to sleep in its ignition-off state it woke up on the OLD level (bench
// switch2, needle stayed at 1/2 after filling 48 -> 100 %).
if ((REFUEL_MODE & 1) && refuelFilling && !refuelWoke && !wakeMuting
    && (EngineIgnitionOn == 1 || millis() - refuelStartAt >= 2000)) {
  wakeMuting = true;
  wakeMuteUntil = millis() + WAKE_MUTE_MS;
  refuelWoke = true;
}
// The cluster wakes up showing the level of that moment (bench rmode1: woke at ~75 % of a
// 10 s fill and stayed there). So keep silent while the level is still rising and wake
// 1 s after the last rise: the needle then jumps to the final level.
if ((REFUEL_MODE & 1) && wakeMuting && refuelWoke && Speed <= 2 && lastRiseAt != 0
    && (long)(lastRiseAt + 1000 - wakeMuteUntil) > 0 && millis() - lastRiseAt < 30000UL) {
  wakeMuteUntil = lastRiseAt + 1000;
}
refuelFull = (REFUEL_MODE & 2) && refuelFilling;
// ETS2 only refuels with the ignition off, and with the ignition off the cluster does not
// follow the level at all; the player then switches on and drives off at once, before a
// wake can finish. With the ignition on and stopped, though, the needle follows a refuel
// in real time (bench 2026-10-04, refuel4). So while the level is rising, tell the cluster
// the ignition is on (0x12F only; everything else still uses the real state).
// Hold it until the real ignition comes on or the truck moves (max REFUEL_IGN_MS after the
// last rise): shown "off" again, the needle fell back to the old level until ignition-on
// (refuel7).
if (Speed > 2 || EngineIgnitionOn == 1) fuelRiseAt = 0;
refuelIgnition = fuelRiseAt != 0 && millis() - fuelRiseAt < REFUEL_IGN_MS;
if (Speed > 2) {
  fuelAtStop = -1;
} else if (fuelAtStop < 0 || fuelpercentage < fuelAtStop) {
  fuelAtStop = fuelpercentage;
} else if (fuelpercentage >= fuelAtStop + 3 && !wakeMuting && EngineIgnitionOn == 1
           && millis() - fuelSteadySince >= FUEL_STEADY_MS) {
  // (ignition off: a wake does not move the needle and even dropped it back to the old
  // level until ignition-on - refuel6. refuelIgnition already moved it during the fill.)
  wakeMuting = true;                       // refuelled while stopped, filling finished
  wakeMuteUntil = millis() + WAKE_MUTE_MS;
  fuelAtStop = fuelpercentage;
}
if (EngineIgnitionOn == 1 && prevIgnitionState == 0) {
  sweepActive = true;
  sweepStartMs = millis();
  if (Speed <= 2) {
    wakeMuting = true;
    wakeMuteUntil = millis() + WAKE_MUTE_MS;
    sweepStartMs = wakeMuteUntil;          // sweep after the cluster woke up
  }
}
prevIgnitionState = EngineIgnitionOn;
if (wakeMuting) {
  // Driving off during the silence latched the l/100km needle at 20 and left the
  // speedo at 0 with "Sanziman. Dikkatli surun" (bench 2026-10-04): end it at once.
  if (Speed <= 2 && (long)(millis() - wakeMuteUntil) < 0) return;   // stay silent on CAN
  wakeMuting = false;
  if (refuelWoke) refuelSpeedHoldUntil = millis() + REFUEL_HOLD_MS;
}
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
if(count == 0x77) {
count = 0;
}
count +=1;
// 0x8A = ignition on ("READY"/running), 0x8 = ignition off ("OFF") - per
// the reference project's sendIgnitionStatus(): "ignitionStatus = ignition ? 0x8A : 0x8".
// Previously hardcoded to 0x8A always, so the tach dial's OFF position never showed.
uint8_t ignitionStatus = (EngineIgnitionOn == 1 || refuelIgnition) ? 0x8A : 0x8;
        unsigned char ignitionWithoutCRC[] = { 0x80 | counter4Bit, ignitionStatus, 0xDD, 0xF1, 0x01, 0x30, 0x06 };
        uint8_t crc = crc8Calculator.get_crc8(ignitionWithoutCRC, 7, 0x44);
        unsigned char ignitionWithCRC[] = { crc, ignitionWithoutCRC[0], ignitionWithoutCRC[1], ignitionWithoutCRC[2], ignitionWithoutCRC[3], ignitionWithoutCRC[4], ignitionWithoutCRC[5], ignitionWithoutCRC[6] };
    CAN.sendMsgBuf(0x12F, 0, 8, ignitionWithCRC);
unsigned char ingandtemp[8] = {0x0, count, count, 0x00, 0x00, count, count, count}; // ignition for F15 cluster

{
int tempForDisplay = inSweepWindow ? 200 : Temp;
ingandtemp[5] = int((0.983607*tempForDisplay) + 51.3169);
CAN.sendMsgBuf(0x3f9, 0, 8, ingandtemp);
}


//if water over 119 then activate engine over heated aleart
if(WTemp >= 119) {

 	      uint8_t engine_overheated[] = { 0x40, 39, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, engine_overheated);
} else {

 	      uint8_t engine_overheated1[] = { 0x40, 39, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, engine_overheated1);
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
// After a refuel wake the needle needs ~3-5 s at standstill to reach the new level; once
// the cluster sees speed it switches to the slow filter (bench rmode1b: stuck at 3/4 when
// driving off 2 s after the wake). So report 0 km/h for REFUEL_HOLD_MS after that wake.
int shownSpeed = ((long)(millis() - refuelSpeedHoldUntil) < 0) ? 0 : Speed;
uint16_t calculatedSpeed = inSweepWindow ? (uint16_t)(260 * 64.01) : (uint16_t)((double)shownSpeed * 64.01); // 260 = dial's printed max
  unsigned char speedWithoutCRC[] = { 0xC0|counter4Bit, lo8(calculatedSpeed), hi8(calculatedSpeed), (shownSpeed == 0 ? 0x81 : 0x91) };
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

// ECO/coasting light: off throttle, moving, engine above idle. idleRPM comes
// from SimHub every packet; it used to grow by 10 every Loop in between (and
// without limit when SimHub stopped sending), a fixed margin does the same job.
if(gas == 0 && Speed > 5 && RPM > idleRPM + 100) {
efficient = 0xF6;
} else {
efficient = 0xF0;
}

// RPM tach message (0xF3), fixed 2026-09-28 with the webcam: byte1 must be
// 0x60 | alive counter and the CRC is over bytes 1-7 (seed 0x7A). The old code
// put the RPM low byte in byte1 and CRC'd the whole frame (first send even used
// the previous message's crc), so only some RPM values happened to be accepted -
// the needle dropped to 0 at e.g. 1500/3000/3500/5000 rpm (the ">2000 rpm cap").
// RPM sits in byte2 at rpm*1.557/256 (~164 rpm per step); swept 0-7000 rpm and
// the needle lands on the printed marks (2000->2, 3000->3, 6000->6). byte3 does
// not add resolution (tested). No smoothing filter: the needle damps itself.
{
  uint16_t rpmRaw = (uint16_t)(RPM2 * 1.557);
  uint8_t rpm_frame[8] = { 0, (uint8_t)(0x60 | counter4Bit), (uint8_t)(rpmRaw >> 8), 0xC0, (uint8_t)efficient, 0x00, 0xFF, 0xFF };
  rpm_frame[0] = crc8Calculator.get_crc8(rpm_frame + 1, 7, 0x7A);
  CAN.sendMsgBuf(0xf3, 0, 8, rpm_frame);
}



//MPG/range bar - byte0 must be a clean incrementing counter (matches the reference project's
//sendDistanceTravelled()) and the CRC poly is 0xC6, not 0xFF. The previous
//`throthel | counter4Bit` byte0 and wrong 0xFF poly produced a checksum that only
//coincidentally matched what the cluster expects, causing the range/consumption bar
//to update inconsistently instead of tracking real fuel usage.
    unsigned char mpgWithoutCRC[] = { (uint8_t)fuelUsed, 0xFF, 0x64, 0x64, 0x64, 0x01, 0xF1 };
    unsigned char mpgWithCRC[] = { crc8Calculator.get_crc8(mpgWithoutCRC, 7, 0xC6), mpgWithoutCRC[0], mpgWithoutCRC[1], mpgWithoutCRC[2], mpgWithoutCRC[3], mpgWithoutCRC[4], mpgWithoutCRC[5], mpgWithoutCRC[6] };
    CAN.sendMsgBuf(0x2C4, 0, 8, mpgWithCRC);
	  

//distance travelld counter
// Gated to a real 100ms interval (matching the reference project's dashboardUpdateTime100),
// unlike the rest of this Loop() which runs unthrottled on every call. Without this
// gate the odometer/distance value inflates far faster than real elapsed time, since
// Loop() itself runs many times faster than 10Hz — this was observed live as the
// cluster's total km jumping ~1000km after only a few minutes of driving.
static unsigned long lastDistanceUpdateMs = 0;
static float distanceFrac = 0;
if (millis() - lastDistanceUpdateMs >= 100) {
    // Scale by the time that really passed: in game, parsing a SimHub packet at
    // 19200 baud makes one Loop take several hundred ms, so "+1 step per 100 ms
    // gate" under-counted distance (and fuel) ~4x (seen on the webcam 2026-09-28:
    // trip +0.2 km in 40 s at 80 km/h). Capped at 1 s after a long gap.
    float ticks = (millis() - lastDistanceUpdateMs) / 100.0;
    if (ticks > 10) ticks = 10;
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
    distanceFrac += Speed * 2.9 * ticks;
    distanceTravelledCounter += (int)distanceFrac;
    distanceFrac -= (int)distanceFrac;
    // 0x2C4 byte0 is the injected-fuel counter (found 2026-09-28 on camera): the
    // cluster's average consumption, its range AND its fuel level estimate all
    // integrate it against 0x2BB distance. The old code sent `count` (+1 every
    // Loop, ~70/s, wrapping at 0x77), far too much fuel: average pinned at the
    // 29.5 display cap and range stuck low. Now it grows with speed x consumption,
    // same 100 ms tick as the distance. Calibrated with the webcam: factor 114
    // makes the MID average show exactly throthel/10 l/100km (10.0 at 40 and 80 km/h).
    // throthel is ETS2 consumption scaled to a car tank (customprotocol field 34),
    // so the cluster's range = ETS2's range. 0 (no data) falls back to 8.0.
    // (2026-10-04) The counter now follows the REAL fuel decrease instead of consumption x
    // speed. The cluster fuses this counter with the 0x349 sender; the two never agreed,
    // so it learned a correction (0x330 showed 5 L with the sender at 99 %) and distrusted
    // refuels. throthel (field 34) = fuel left in a 52 L car tank, centilitres
    // (FuelPercent x 52). 1 L = 4104 counts (from the factor-114 calibration above), so
    // 1 cL = 41 counts. Fed in at most 40 counts per tick so the byte never jumps.
    {
      static int lastCl = -1;
      static float pendingCounts = 0;
      if (throthel > 0) {
        if (lastCl > 0 && throthel < lastCl && lastCl - throthel < 200)   // < 2 L: real use
          pendingCounts += (lastCl - throthel) * 41.04;
        lastCl = throthel;                                                 // rises = refuel
      }
      float step = pendingCounts < 40.0 * ticks ? pendingCounts : 40.0 * ticks;
      pendingCounts -= step;
      fuelUsed += step;
      while (fuelUsed >= 256.0) fuelUsed -= 256.0;
    }
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
  uint8_t fuelQuantityLiters = multiMap<uint8_t>(refuelFull ? 100 : fuelpercentage, inFuelRange, outFuelRange, 6);
  // (2026-09-23) No forced-empty test window anymore - see the removal note near
  // the top of Loop() for why. This is always just the real computed value now.
  // The gauge is still known to be heavily damped/slow inside the cluster's own
  // hardware (see the older fuel-sweep-speed entries in CLAUDE.md) - that part is
  // untouched and still applies.
  unsigned char fuelWithoutCRC[] = { (isCarMini ? 0 : hi8(fuelQuantityLiters)), (isCarMini ? 0 : lo8(fuelQuantityLiters)), hi8(fuelQuantityLiters), lo8(fuelQuantityLiters), 0x00 };
  CAN.sendMsgBuf(0x349, 0, 5, fuelWithoutCRC);
  
  
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

// ETS2 has no P. Neutral + parking brake + standing still = parked, so show P
// (user request 2026-10-04).
if (selectedGear == 0x60 && handbrake == 1 && Speed <= 1) {
selectedGear = 0x20;
manual = counter4Bit;
}

// N while rolling raises "Sanziman. Dikkatli surun" (CC 173): bench shift3, D->N at
// 30 km/h, and the user in ETS2 (D->N while moving; stopping first is fine). A real car
// would also need a brake pedal signal we don't know yet. Keep showing D until stopped.
if (selectedGear == 0x60 && Speed > 3) {
  selectedGear = 0x80;
}

// Like a real BMW automatic: plain D in the normal modes, S1..S9 only in the sport
// modes (user request 2026-10-04). drivemod 4 = Sport, 5 = Sport+, 6 = DSC off.
if (selectedGear == 0x81 && !(drivemod == 4 || drivemod == 5 || drivemod == 6)) {
  selectedGear = 0x80;
}

// byte0 = gear number (high nibble, shown as S1..S9) | alive counter (low nibble).
// The branches above took the low nibble from different counters (gmax 0-8, the last
// gear + gmax for 20 loops in N, counter4Bit 0-13 otherwise), so it jumped on every
// shift and at start-up in N: the cluster raised "Sanziman. Dikkatli surun" (CC 173)
// - user saw it when shifting in ETS2, bench gear1 had it from the first N frame.
// One continuous counter for every gear now.
{
  uint8_t gearNum = (selectedGear == 0x81) ? (uint8_t)(gear.toInt() & 0x0F) : 0;
  manual = (gearNum << 4) | (counter4Bit & 0x0F);
}
    unsigned char transmissionWithoutCRC[] = { manual, selectedGear, 0xFC, 0xFF }; //0x20= P, 0x40= R, 0x60= N, 0x80= D, 0x81= DS
    unsigned char transmissionWithCRC[] = { crc8Calculator.get_crc8(transmissionWithoutCRC, 4, 0xD6), transmissionWithoutCRC[0], transmissionWithoutCRC[1], transmissionWithoutCRC[2], transmissionWithoutCRC[3] };
    CAN.sendMsgBuf(0x3FD, 0, 5, transmissionWithCRC);
    
    
//check engine
if(checkengwhenoff == false) {
EngineIgnitionOn = 1;
}
  if(oilpress == 1 or EngineIgnitionOn == 0 or (tempomats & 0x01)) {   // bit0: ETS2 any damage >= 15 %
      uint8_t message2[] = { 0x40, 34, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, message2);
} else {
	      uint8_t message2[] = { 0x40, 34, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, message2);
}


// Kept hardcoded off deliberately: code 71 was confirmed (2026-08-28) to also
// toggle the parking-brake lamp on its own, unlike code 77 below which is clean.
// EXPLAINED (2026-08-29): The reference project labels code 71 as
// "Park brake error (red)", NOT seatbelt - this project's original variable name
// ("seat_belt_indecator") was simply wrong. See CLAUDE.md before ever driving this
// one from the switch.
uint8_t seat_belt_indecator[] = { 0x40, 71, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
CAN.sendMsgBuf(0x5c0, 0, 8, seat_belt_indecator);



//dscwarning
if(dscWarning == 1) {
      uint8_t dscWarning[] = { 0x40, 215, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, dscWarning);
} else {
	      uint8_t dscWarning1[] = { 0x40, 215, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, dscWarning1);
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
      CAN.sendMsgBuf(0x5c0, 0, 8, brakeAirWarning);
} else {
      uint8_t brakeAirWarning1[] = { 0x40, 24, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, brakeAirWarning1);
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
CAN.sendMsgBuf(0x5c0, 0, 8, Seatbeltlamp); //driven by real buckle switch, see above


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


//fog / high beam / lights-on indicators (0x21A byte0)
// Bit map from a full webcam sweep of byte0 (2026-09-28, docs/RESEARCH.md):
// 0x02 blue high beam, 0x04 green "lights on" icon AND the night (red) MID text
// theme - the two are one bit and can't be separated (byte1 and 0x202 byte1 were
// swept too, neither changes the theme), 0x20 front fog, 0x40 rear fog,
// 0x01/0x08/0x10/0x80 no visible effect. There is no separate low-beam icon on
// this cluster. The values below are the same ones the old if/else chain sent
// (0x05 lights, 0x07 high, 0x60 fog, ...); nightThemeWithLights = false only
// masks 0x04 so the text stays white with the lights on, and lightsIconViaFog
// then borrows the green front-fog icon (0x20, keeps white text) as the
// "lights on" indicator.
{
uint8_t lightsByte = 0;
if (light == 1 || highbeam == 1) lightsByte |= 0x05;
if (highbeam == 1) lightsByte |= 0x02;
if (fogg == 1) lightsByte |= 0x60;
if (!nightThemeWithLights) {
  lightsByte &= ~0x04;
  if (lightsIconViaFog && (light == 1 || highbeam == 1)) lightsByte |= 0x20;
}
uint8_t lightsFrame[] = { lightsByte, 0x00, 0xf7 };
CAN.sendMsgBuf(0x21A, 0, 3, lightsFrame);
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
if (backlightAlwaysOn || light == 1 || showLights == 1) {
    uint8_t mappedBrightness = map(backlight, 0, 100, 0, 253);
    unsigned char backlightBrightnessWithoutCRC[] = { mappedBrightness, 0xFF };
    CAN.sendMsgBuf(0x202, 0, 2, backlightBrightnessWithoutCRC);
} else {
    unsigned char backlightOffWithoutCRC[] = { 0, 0xFF };
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

// Clock/date: CAN 0x39E "set time and date" (the message a head unit uses to set
// the cluster's own clock), confirmed on camera 2026-09-28: one frame set the MID
// clock and date, and cleared the yellow "set date/time" triangle. Layout from
// E87/E90 captures: hour, minute, second, day, (month<<4)|0x0F, year lo, year hi,
// 0xF2, no CRC. The cluster keeps time itself afterwards, so this is sent once
// when valid PC time first arrives from SimHub, then every 30 s (so a cluster that lost 12 V while the Arduino stayed on USB gets the time back quickly) to stay in
// sync (no need to stream it). The old 0x2F8 attempt is gone: 0x2F8 is the
// cluster's own broadcast in the E-series DBC, not an input.
// Fields: H=hour(35), absWarning=minute(16), dscSwitch=second(18),
// pcars_mcarflags=day(20), acc_flashlight=month(22), checkEngine_ETS=year(5).
{
  static unsigned long lastClockSetMs = 0;
  static bool clockSet = false;
  bool validTime = checkEngine_ETS >= 2020 && acc_flashlight >= 1 && acc_flashlight <= 12 && pcars_mcarflags >= 1;
  if (validTime && (!clockSet || millis() - lastClockSetMs >= 30000UL)) {
    uint16_t clockYear = (uint16_t)checkEngine_ETS;
    uint8_t setTime[] = { (uint8_t)H, (uint8_t)absWarning, (uint8_t)dscSwitch, (uint8_t)pcars_mcarflags,
                          (uint8_t)((acc_flashlight << 4) | 0x0F), lo8(clockYear), hi8(clockYear), 0xF2 };
    CAN.sendMsgBuf(0x39E, 0, 8, setTime);
    clockSet = true;
    lastClockSetMs = millis();
  }
}

 
// ETS2 warnings -> check-control messages (customprotocol field 45 bitmask):
//   bit0 any damage >= 15 %     -> engine MIL (handled in the check-engine block)
//   bit1 any damage >= 20 %     -> 29  "Tahrik. Dikkatli surun"   (10 s popup)
//   bit2 any damage >= 40 %     -> 41  "Servis gerekli"            (10 s popup)
//   bit3 engine damage >= 50 %  -> 170 "Motor arizali"             (10 s popup)
//   bit4 differential lock      -> 780 "Arka aks kilitli diferansiyel" (while on)
//   bit5 speed limit + 5 km/h   -> no longer shown (was 768 "LIM", too big); speed
//                                  > 100 / > 120 km/h give 3 s popups 78 / 62 instead
//   bit7 average wheel wear >= 25 % -> 265 "Lastik basincini kontrol edin" (10 s)
//   bit6 trailer attached       -> 858 "Romork tanimlandi" on attach, 75 "Romork
//                                  baglantisi elektrigi" on detach (8 s popups)
// Only one code is ON at a time (several ON codes at once confused the cluster,
// see docs/HISTORY.md); a replaced code gets explicit OFF frames.
{
  static uint8_t prevBits = 0;
  static unsigned long popupUntil[3] = { 0, 0, 0 };
  static unsigned long trailerUntil = 0;
  static int trailerCode = -1;
  static int shownCode = -1;
  static int offCode = -1;
  static uint8_t offRepeats = 0;
  const uint8_t popBit[3] = { 0x08, 0x02, 0x04 };       // priority: 170, 29, 41
  const int popCode[3] = { 170, 29, 41 };
  uint8_t bits = (EngineIgnitionOn == 1) ? (uint8_t)tempomats : 0;
  for (uint8_t i = 0; i < 3; i++)
    if ((bits & popBit[i]) && !(prevBits & popBit[i])) popupUntil[i] = millis() + 10000;
  if ((bits ^ prevBits) & 0x40) { trailerCode = (bits & 0x40) ? 858 : 75; trailerUntil = millis() + 8000; }
  prevBits = bits;
  // Speed warnings (user request 2026-10-04, replaces the permanent 768 "LIM" popup):
  // crossing 100 km/h -> 78 yellow "! Hiz uyarisi" for 3 s, crossing 120 km/h -> 62 red
  // "! Hiz uyarisi" for 3 s. 5 km/h hysteresis so it doesn't flicker at the threshold.
  static bool over100 = false, over120 = false;
  static unsigned long speedWarnUntil = 0;
  static int speedWarnCode = -1;
  if (EngineIgnitionOn == 1 && !over120 && Speed > 120) { over120 = over100 = true; speedWarnCode = 62; speedWarnUntil = millis() + 3000; }
  else if (EngineIgnitionOn == 1 && !over100 && Speed > 100) { over100 = true; speedWarnCode = 78; speedWarnUntil = millis() + 3000; }
  if (Speed < 115) over120 = false;
  if (Speed < 95) over100 = false;
  // Fuel reserve (user request 2026-10-04): below 15 % (ETS2's own fuel warning level)
  // -> 275 "Yakit rezervi" for 10 s; again only after going back above 20 %.
  // Tyres: ETS2 has no per-wheel pressure, only the average wheel wear (field 45 bit7,
  // WheelsAvg >= 25 %) -> 265 "Lastik basincini kontrol edin" for 10 s when it starts.
  static bool lowFuel = false;
  static unsigned long fuelWarnUntil = 0, tyreWarnUntil = 0;
  if (EngineIgnitionOn == 1 && !lowFuel && fuelpercentage > 0 && fuelpercentage < 15) { lowFuel = true; fuelWarnUntil = millis() + 10000; }
  if (fuelpercentage > 20) lowFuel = false;
  static bool prevTyre = false;
  bool tyre = bits & 0x80;
  if (tyre && !prevTyre) tyreWarnUntil = millis() + 10000;
  prevTyre = tyre;
  int want = -1;
  if (speedWarnCode >= 0 && (long)(speedWarnUntil - millis()) > 0) want = speedWarnCode;
  if (want < 0 && (long)(fuelWarnUntil - millis()) > 0) want = 275;
  if (want < 0 && (long)(tyreWarnUntil - millis()) > 0) want = 265;
  if (want < 0 && trailerCode >= 0 && (long)(trailerUntil - millis()) > 0) want = trailerCode;
  for (uint8_t i = 0; i < 3 && want < 0; i++)
    if ((bits & popBit[i]) && (long)(popupUntil[i] - millis()) > 0) want = popCode[i];
  if (want < 0 && (bits & 0x10)) want = 780;
  if (want != shownCode) {
    if (shownCode >= 0) { offCode = shownCode; offRepeats = 20; }
    shownCode = want;
  }
  if (offCode >= 0 && offRepeats > 0) {
    uint8_t f[] = { 0x40, (uint8_t)(offCode & 0xFF), (uint8_t)(offCode >> 8), 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
    CAN.sendMsgBuf(0x5c0, 0, 8, f);
    if (--offRepeats == 0) offCode = -1;
  }
  if (shownCode >= 0) {
    uint8_t f[] = { 0x40, (uint8_t)(shownCode & 0xFF), (uint8_t)(shownCode >> 8), 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
    CAN.sendMsgBuf(0x5c0, 0, 8, f);
  }
}

//tire flat
if (tfl == 1) {
  uint8_t tfld[] = { 0x40, 0x8B, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, tfld);
} else {
  uint8_t tfld[] = { 0x40, 0x8B, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, tfld);
 }

if (tfr == 1) {
  uint8_t tfrd[] = { 0x40, 0x8F, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, tfrd);
} else {
  uint8_t tfrd[] = { 0x40, 0x8F, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, tfrd);
 }
 
 if (trl == 1) {
  uint8_t trld[] = { 0x40, 0x8D, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, trld);
} else {
  uint8_t trld[] = { 0x40, 0x8D, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, trld);
 }
 
 if (trr == 1) {
  uint8_t trrd[] = { 0x40, 0x8C, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, trrd);
} else {
  uint8_t trrd[] = { 0x40, 0x8C, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, trrd);
 }


//doors, trunk, frunk open or closed
if (dfl == 1) {
  uint8_t dfld[] = { 0x40, 0xF, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, dfld);
} else {
  uint8_t dfld[] = { 0x40, 0xF, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, dfld);
 }

if (dfr == 1) {
  uint8_t dfrd[] = { 0x40, 0xE, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, dfrd);
} else {
  uint8_t dfrd[] = { 0x40, 0xE, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, dfrd);
 }
 
 if (drl == 1) {
  uint8_t drld[] = { 0x40, 0x10, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, drld);
} else {
  uint8_t drld[] = { 0x40, 0x10, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, drld);
 }
 
 if (drr == 1) {
  uint8_t drrd[] = { 0x40, 0x11, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, drrd);
} else {
  uint8_t drrd[] = { 0x40, 0x11, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, drrd);
 }

 if (hood == 1) {
  uint8_t hoodd[] = { 0x40, 0x12, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, hoodd);
} else {
  uint8_t hoodd[] = { 0x40, 0x12, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, hoodd);
 }
 
 if (trunk == 1) {
  uint8_t trunkd[] = { 0x40, 0x13, 0x00, 0x29, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, trunkd);
} else {
  uint8_t trunkd[] = { 0x40, 0x13, 0x00, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
      CAN.sendMsgBuf(0x5c0, 0, 8, trunkd);
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
