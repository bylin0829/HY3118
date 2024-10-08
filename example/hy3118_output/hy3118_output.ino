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
// Function prototype
HY3118 adc(HW_ID);
void hy3118Config();

// Variable
uint8_t count = 0;
uint32_t adc_avg = 0, adc_max, adc_min;
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Serial.println("adc testing");
  hy3118Config();
  adc.tare();
  adc_max = adc.getSmoothedData();
  adc_min = adc.getSmoothedData();
}

void loop()
{
  char buffer[100];
  uint32_t adc_data = (uint32_t)adc.getSmoothedData();
  if (adc_data > adc_max)
  {
    adc_max = adc_data;
  }
  if (adc_data < adc_min)
  {
    adc_min = adc_data;
  }
  sprintf(buffer, "adc:%ld, max:%ld, min:%ld, diff:%ld\n", adc_data, adc_max, adc_min, adc_max - adc_min);
  Serial.print(buffer);
  delay(500);
}

void hy3118Config()
{
  adc.begin();
  adc.REG_0(APO_DISABLE, IRQEN_DISABLE, ENADC_ENABLE, ENLDO_ENABLE, ENREFO_ENABLE, ENOP_DISABLE);
  adc.REG_1(AIN1, AIN2);
  adc.REG_2(VDDA, VSSA, VREF_0);
  adc.REG_3(Internal327KHz, Full, Gain32, Gain4);
  adc.REG_4(ldo_3v3, refo_1v5, Slow_327kHz, Rate10_30);

  char buffer[30];
  for (int i = 0; i < 5; i++)
  {
    sprintf(buffer, "B[%d]: %xH ", i, adc.readRegister(i));
    Serial.print(buffer);
  }

  Serial.println("");
}