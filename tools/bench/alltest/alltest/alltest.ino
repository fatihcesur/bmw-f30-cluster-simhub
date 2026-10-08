// Standalone bench test: no SimHub needed. Wakes the cluster and turns on every
// indicator the main firmware knows: ignition, backlight, hazards, high beam + fog,
// parking brake, cruise, check-engine (34) and seatbelt (77). Needles sit at
// 100 km/h, 5000 rpm, fuel 75 %. Frames are copied from firmware/bmw_f30_cluster/SHCustomProtocol.h.
//
// Built-in LED: slow blink = CAN OK and sending, fast blink = MCP2515 init failed.
// Serial 115200 prints the CAN init result and a send/error counter every second.

#include <SPI.h>
#include <mcp_can.h>
#include "CRC8.h"

#define lo8(x) ((int)(x) & 0xff)
#define hi8(x) ((int)(x) >> 8)

MCP_CAN CAN(10);
CRC8 crc8;
bool canOk = false;
uint8_t ctr = 0;
unsigned long okCount = 0, errCount = 0;

void tx(unsigned long id, uint8_t len, uint8_t *buf) {
  if (CAN.sendMsgBuf(id, 0, len, buf) == CAN_OK) okCount++; else errCount++;
}

void txCrc(unsigned long id, uint8_t *body, uint8_t n, uint8_t xorOut) {
  uint8_t f[8];
  f[0] = crc8.get_crc8(body, n, xorOut);
  for (uint8_t i = 0; i < n; i++) f[i + 1] = body[i];
  tx(id, n + 1, f);
}

void cc(uint8_t code, bool on) {
  uint8_t f[] = { 0x40, code, 0x00, (uint8_t)(on ? 0x29 : 0x28), 0xFF, 0xFF, 0xFF, 0xFF };
  tx(0x5c0, 8, f);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, INPUT);
  crc8.begin();
  for (uint8_t tries = 0; tries < 5 && !canOk; tries++) {
    canOk = CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK;
    if (!canOk) delay(200);
  }
  if (canOk) CAN.setMode(MCP_NORMAL);
  Serial.println(canOk ? F("ALLTEST: MCP2515 OK") : F("ALLTEST: MCP2515 INIT FAILED"));
}

void loop() {
  static unsigned long lastFrame = 0, lastDist = 0, lastLog = 0, lastLed = 0;
  static uint16_t dist = 0;
  static bool led = false;
  unsigned long now = millis();

  if (now - lastLed >= (canOk ? 500UL : 100UL)) {
    lastLed = now;
    led = !led;
    digitalWrite(LED_BUILTIN, led);
  }
  if (!canOk) return;

  if (now - lastLog >= 1000) {
    lastLog = now;
    Serial.print(F("ok=")); Serial.print(okCount);
    Serial.print(F(" err=")); Serial.print(errCount);
    Serial.print(F(" TEC=")); Serial.print(CAN.errorCountTX());
    Serial.print(F(" REC=")); Serial.println(CAN.errorCountRX());
  }

  if (now - lastFrame < 100) return;
  lastFrame = now;
  ctr = (ctr + 1) & 0x0F;

  // Ignition on (0x12F)
  { uint8_t b[] = { (uint8_t)(0x80 | ctr), 0x8A, 0xDD, 0xF1, 0x01, 0x30, 0x06 }; txCrc(0x12F, b, 7, 0x44); }
  // F15-style ignition + oil temp (0x3F9)
  { uint8_t b[] = { 0, ctr, ctr, 0, 0, 150, ctr, ctr }; tx(0x3F9, 8, b); }
  // Speed 100 km/h (0x1A1)
  { uint16_t s = (uint16_t)(100 * 64.01);
    uint8_t b[] = { (uint8_t)(0xC0 | ctr), (uint8_t)lo8(s), (uint8_t)hi8(s), 0x91 }; txCrc(0x1A1, b, 4, 0xA9); }
  // RPM 5000 (0xF3)
  { uint16_t r = (uint16_t)(5000 * 1.557);
    uint8_t f[8] = { 0, (uint8_t)(0x60 | ctr), (uint8_t)(r >> 8), 0xC0, 0xF0, 0x00, 0xFF, 0xFF };
    f[0] = crc8.get_crc8(f + 1, 7, 0x7A); tx(0xF3, 8, f); }
  // Fuel 75 % (0x349)
  { uint8_t l = 8; uint8_t b[] = { 0, l, 0, l, 0 }; tx(0x349, 5, b); }
  // Hazards (0x1F6): both sides
  { uint8_t b[] = { (uint8_t)(0x81 | 1 << 4 | 1 << 5), 0xF0 }; tx(0x1F6, 2, b); }
  // High beam + fog + lights (0x21A)
  { uint8_t b[] = { 0x67, 0x00, 0xF7 }; tx(0x21A, 3, b); }
  // Backlight full (0x202)
  { uint8_t b[] = { 253, 0xFF }; tx(0x202, 2, b); }
  // Parking brake on (0x34F)
  { uint8_t b[] = { 0xFE, 0xFF }; tx(0x34F, 2, b); }
  // Gear D (0x3FD)
  { uint8_t b[] = { ctr, 0x80, 0xFC, 0xFF }; txCrc(0x3FD, b, 4, 0xD6); }
  // Drive mode Sport (0x3A7)
  { uint8_t b[] = { (uint8_t)(0xF0 | ctr), 0, 0, 4, 0x11, 0xC0 }; txCrc(0x3A7, b, 6, 0x4A); }
  // Cruise on (0x289)
  { uint8_t b[] = { (uint8_t)(0xF0 | ctr), 0x00, 0xE0, 0xE1, 0x96, 0x14, 0x00 }; txCrc(0x289, b, 7, 0x82); }
  // Keep-alives the cluster expects (ABS, steering column, airbag, safety)
  { uint8_t b[] = { (uint8_t)(0xF0 | ctr), 0xFE, 0xFF, 0x14 }; txCrc(0x36E, b, 4, 0xD8); }
  { uint8_t b[] = { (uint8_t)(0xF0 | ctr), 0xFE, 0xFF, 0x14 }; txCrc(0x2A7, b, 4, 0x9E); }
  { uint8_t b[] = { ctr, 0xFF }; tx(0xD7, 2, b); }
  { uint8_t b[] = { (uint8_t)(0x40 | ctr), 0x40, 0x55, 0xFD, 0xFF, 0xFF, 0xFF }; txCrc(0x19B, b, 7, 0xFF); }
  { uint8_t b[] = { (uint8_t)(0xE0 | ctr), 0xF1, 0xF0, 0xF2, 0xF2, 0xFE }; txCrc(0x297, b, 6, 0x28); }
  // Language Turkish, C, l/100km, km (0x291)
  { uint8_t b[] = { 0, 18, 89, 0, 0, 0, 0, 0 }; tx(0x291, 8, b); }
  // Distance counter (0x2BB), 100 ms gated like the main firmware
  if (now - lastDist >= 100) {
    lastDist = now;
    uint8_t b[] = { (uint8_t)(0xF0 | ctr), (uint8_t)lo8(dist), (uint8_t)hi8(dist), 0xF2 };
    txCrc(0x2BB, b, 4, 0xDE);
    dist += (uint16_t)(100 * 2.9);
  }
  // Check-control lamps, same set and pattern the main firmware sends every loop
  cc(34, true);   // check engine
  cc(77, true);   // seatbelt
  cc(71, false);
  cc(215, false);
  cc(24, false);
  cc(39, false);
}
