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

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Serial.println("HY3118 program 0909 21:39");
  hy3118Config();
  adc.tare();
}

uint8_t count = 0;
uint32_t adc_avg = 0;
void loop()
{
  uint32_t adc_data = adc.readADC();
  if (adc_data & 0x1)
  { // bit 0 = adc ready or not
    adc_data >>= 1;
    count++;
    adc_avg += adc_data;
    // Serial.println(adc_data, DEC);
    if (count >= 10)
    {
      Serial.print("Avg=");
      Serial.println((int)(adc_avg / 10), DEC);
      count = 0;
      adc_avg = 0;
    }
  }
  delay(100);
}
/*
0kg = 138200
(5400)
0.5kg = 132800
(4400)
1kg 128400
*/

void hy3118Config()
{
  adc.begin();
  adc.REG_0(false, false, true, true, true, true);
  adc.REG_1(AIN1, AIN2);
  adc.REG_2(Vrefp_buffered, Vrefn_buffered, VREF_0);
  adc.REG_3(Internal327KHz, Full, Gain32, Gain4);
  adc.REG_4(ldo_3v3, refo_1v5, Slow_327kHz, Rate10_30);
  // B[0]: 1Ch  B[1]: 8h  B[2]: 0h  B[3]: 1Fh  B[4]: 0h

  char buffer[30];
  for (int i = 0; i < 5; i++)
  {
    sprintf(buffer, "B[%d]: %xH ", i, adc.readRegister(i));
    Serial.print(buffer);
  }
  Serial.println("");
}

const int sample_weight = 50;

void calibration()
{
  float scale_factor = (current_weight / sample_weight);
  Serial.print("Scale number:  ");
  Serial.println(scale_factor, 0); // 顯示比例參數，記起來，以便用在正式的程式中
}