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
float data;
float knownWeight = 2.0; // unit: "kg"
float factor;
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Serial.println("[HY3118 Calibration]");
  hy3118Config();

  // [Set zero offset]
  // remove all of item on load cell
  Serial.println("[Set zero offset]");
  Serial.println("Remove weight on load cell");
  Serial.println("Keyin 'y' to continue.");
  while (Serial.read() != 'y')
    ;

  // read adc value at least SAMPLE*2 times then setting zero offset.
  for (int i = 0; i < (SAMPLES * 5); i++)
  {
    Serial.print("Sampling ");
    Serial.print(i);
    Serial.print(", ");
    Serial.println(adc.getSmoothedData());
  }

  // set zero offset (Tare)
  adc.setTareOffset(adc.getSmoothedData());
  Serial.print("Set zero offset:");
  Serial.println(data);
  Serial.print("Get zero offset:");
  Serial.println(adc.getTareOffset());

  // [Known weight]
  Serial.println("[Put known mass]");
  Serial.print("Weight(kg):");
  Serial.println(knownWeight);
  Serial.println("Keyin 'y' to continue.");
  while (Serial.read() != 'y')
    ;

  // read weight at least 60 times.
  for (int i = 0; i < (SAMPLES * 5); i++)
  {
    Serial.print("Sampling ");
    Serial.print(i);
    Serial.print(", ");
    Serial.println(adc.getSmoothedData());
  }
  // write calibration weight
  factor = (adc.getSmoothedData() - (float)adc.getTareOffset()) / knownWeight;
  adc.setCalFactor(factor);
  Serial.print("Set factor:");
  Serial.println(factor);
  Serial.print("Get factor:");
  Serial.println(adc.getCalFactor());

  // continuous reading
  delay(3000);
}

void loop()
{
  Serial.print("factor= ");
  Serial.print(factor);
  Serial.print(", ");
  Serial.print(adc.getWeight(SAMPLES));
  Serial.println(" kg");
  delay(100);
}

void hy3118Config()
{
  adc.begin();

  adc.REG_0(APO_DISABLE, IRQEN_DISABLE, ENADC_ENABLE, ENLDO_ENABLE, ENREFO_ENABLE, ENOP_DISABLE);
  adc.REG_1(AIN1, AIN2);
  adc.REG_2(VDDA, VSSA, VREF_0);
  adc.REG_3(Internal327KHz, Full, Gain32, Gain4);
  adc.REG_4(ldo_3v3, refo_1v5, Slow_327kHz, Rate10_30);
}

/*
zero offset = 100
400g = 900
calFactor = 900
factorRecip = 1/900

ret = (900-100)/900


----other example----
const int sample_weight = 150;  //基準物品的真實重量(公克)
float current_weight=scale.get_units(10);  // 取得10次數值的平均
float scale_factor=(current_weight/sample_weight);

scale_factor = 900/150 = 6

900
*/