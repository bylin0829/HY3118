// --------------------------------------
// i2c_scanner
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
// ---------------------------------------

/*

 Example guide:
 https://www.amebaiot.com/en/amebad-arduino-i2c-scan/
 */

#include <Wire.h>
#include "HY3118.h"

#define HW_ID 0x50
HY3118 adc(HW_ID);
void hy3118Config();

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // Leonardo: wait for serial monitor

  hy3118Config();  // set HY3118
  while (!adc.getSmoothedDataStatus()) {
    Serial.println(adc.getSmoothedData());
  }  // wait for smoothed data sampling done
  Serial.println("Smooth flow is done");
  adc.tare();
  Serial.println(adc.getTareOffset());
  delay(3000);
}

void loop() {
  Serial.print(adc.getTareOffset());
  Serial.print("  ");
  Serial.println(adc.getSmoothedData());
  delay(100);
}

void hy3118Config() {
  adc.begin();

  adc.REG_0(false, false, true, true, true, true);
  adc.REG_1(AIN1, AIN2);
  adc.REG_2(Vrefp_buffered, Vrefn_buffered, VREF_0);
  adc.REG_3(Internal327KHz, Full, Gain32, Gain4);
  adc.REG_4(ldo_3v3, refo_1v5, Slow_327kHz, Rate10_30);
}