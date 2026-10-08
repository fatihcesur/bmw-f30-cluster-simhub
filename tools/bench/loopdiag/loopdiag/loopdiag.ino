#include <SPI.h>
#include <mcp_can.h>
MCP_CAN CAN(10);
void setup() {
  Serial.begin(115200); delay(300);
  Serial.println(CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK ? F("begin OK") : F("begin FAIL"));
  CAN.setMode(MCP_LOOPBACK);
  uint8_t b[8] = {1,2,3,4,5,6,7,8}; int sent = 0, got = 0;
  for (int i = 0; i < 20; i++) {
    b[0] = i;
    if (CAN.sendMsgBuf(0x123, 0, 8, b) == CAN_OK) sent++;
    delay(5);
    while (CAN.checkReceive() == CAN_MSGAVAIL) { unsigned long id; uint8_t len, r[8]; CAN.readMsgBuf(&id, &len, r); if (id == 0x123) got++; }
  }
  Serial.print(F("LOOPBACK sent=")); Serial.print(sent); Serial.print(F(" got=")); Serial.println(got);
  CAN.setMode(MCP_NORMAL); delay(50);
  int ok = 0;
  for (int i = 0; i < 20; i++) if (CAN.sendMsgBuf(0x123, 0, 8, b) == CAN_OK) ok++;
  Serial.print(F("NORMAL ok=")); Serial.print(ok); Serial.print(F(" TEC=")); Serial.print(CAN.errorCountTX());
  Serial.print(F(" REC=")); Serial.print(CAN.errorCountRX()); Serial.print(F(" EFLG=0x")); Serial.println(CAN.getError(), HEX);
}
void loop() {}
