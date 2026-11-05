# Arduino libraries

Install these with the Arduino IDE library manager or from the links:

| Library | Link |
|---|---|
| Adafruit GFX | https://github.com/adafruit/Adafruit-GFX-Library |
| Adafruit BusIO | https://github.com/adafruit/Adafruit_BusIO |
| MCP_CAN (coryjfowler) | https://github.com/coryjfowler/MCP_CAN_lib |
| MultiMap | https://www.arduino.cc/reference/en/libraries/multimap/ |
| X9C10X | https://github.com/RobTillaart/X9C10X |

Board: **Arduino Nano** (`arduino:avr:nano`). If upload fails with `not in sync`, try the
"ATmega328P (Old Bootloader)" processor option.

STM32 Blue Pill: install the **STMicroelectronics STM32** core (board manager URL`https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`), FQBN `STMicroelectronics:stm32:GenF1:pnum=BLUEPILL_F103C8,usb=CDCgen,upload_method=swdMethod`.
