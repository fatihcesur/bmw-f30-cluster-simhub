#include <SPI.h>
#include <mcp_can.h>
MCP_CAN CAN(10);
void tryCfg(uint8_t spd, uint8_t clk, const __FlashStringHelper *name) {
  if (CAN.begin(MCP_ANY, spd, clk) != CAN_OK) { Serial.print(name); Serial.println(F(" begin FAIL")); return; }
  CAN.setMode(MCP_LISTENONLY);
  unsigned long t = millis(), n = 0; long id; uint8_t len, buf[8]; unsigned long ids[6] = {0}; uint8_t k = 0;
  while (millis() - t < 2000) {
    if (CAN.checkReceive() == CAN_MSGAVAIL) { CAN.readMsgBuf((unsigned long*)&id, &len, buf); n++; if (k < 6) ids[k++] = id; }
  }
  Serial.print(name); Serial.print(F(" frames=")); Serial.print(n);
  Serial.print(F(" REC=")); Serial.print(CAN.errorCountRX());
  Serial.print(F(" EFLG=0x")); Serial.print(CAN.getError(), HEX);
  Serial.print(F(" ids:")); for (uint8_t i = 0; i < k; i++) { Serial.print(' '); Serial.print(ids[i] & 0x7FF, HEX); }
  Serial.println();
}
void setup() { Serial.begin(115200); delay(500); }
void loop() {
  tryCfg(CAN_500KBPS, MCP_8MHZ, F("500k@8MHz"));
  tryCfg(CAN_500KBPS, MCP_16MHZ, F("500k@16MHz"));
  tryCfg(CAN_250KBPS, MCP_8MHZ, F("250k@8MHz"));
  tryCfg(CAN_1000KBPS, MCP_8MHZ, F("1M@8MHz"));
  tryCfg(CAN_100KBPS, MCP_8MHZ, F("100k@8MHz"));
  Serial.println(F("---"));
}
