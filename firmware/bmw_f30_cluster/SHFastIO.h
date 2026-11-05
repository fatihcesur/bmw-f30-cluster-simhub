#ifndef __SHFASTIO_H__
#define __SHFASTIO_H__
#include <Arduino.h>

class FastDigitalPin {
private:
#if defined(__AVR__)
	uint8_t bit = 0;
	uint8_t port = 0;
#else
	int pinNo = -1;           // non-AVR (STM32): plain digitalRead
#endif
	bool pinIsValid = false;
public:

	bool isValid() {
		return pinIsValid;
	}
	void begin(int pin) {
		if (pin < 0) {
			pinIsValid = false;
		}
		else {
			pinIsValid = true;
#if defined(__AVR__)
			bit = digitalPinToBitMask(pin);
			port = digitalPinToPort(pin);
#else
			pinNo = pin;
#endif
		}
	}

	int digitalRead()
	{
#if defined(__AVR__)
		if (*portInputRegister(port) & bit) return HIGH;
		return LOW;
#else
		return ::digitalRead(pinNo);
#endif
	}
};

#endif
