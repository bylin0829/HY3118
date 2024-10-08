#include "Arduino.h"
#include "Wire.h"
#include "HY3118.h"
#include <stdio.h>
#include <stdlib.h>

#define HY3118_DBG 0

HY3118::HY3118(uint8_t address) : _address(address) {}

void HY3118::begin()
{
    Wire.begin();
}

void HY3118::writeRegister(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t HY3118::readRegister(uint8_t reg)
{
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission(true);
    // Wire.endTransmission(false);

    Wire.requestFrom(_address, (size_t)1);
    if (Wire.available())
    {
        return Wire.read();
    }
    else
    {
        return 0; // Read data failed
    }
}

void HY3118::updateRawData()
{
    Wire.beginTransmission(_address);
    Wire.write(0x5);
    Wire.endTransmission(true);

    long myTime = millis();
    readDataTimeoutFlag = 0;
    isRawDataReady = 0;
    while (isRawDataReady == 0)
    {
        Wire.requestFrom(_address, (size_t)3);
        uint32_t tempAdcVal = 0;
        int i = 2;

        while (Wire.available() > 0)
        {
            uint32_t temp = Wire.read();
            tempAdcVal |= (temp << (i * 8));
            i--;
        }

        if (tempAdcVal & 0x01) // Check if the data is read
        {
            adcRawData = tempAdcVal >> 1;
            isRawDataReady = 1;
            break;
        }
        else if (millis() - myTime > 5000)
        {
            readDataTimeoutFlag = 1;
            break;
        }
    }
}

void HY3118::REG_0(bool apo, bool irqen, bool enadc, bool enldo, bool enrefo, bool enop)
{
    uint8_t data =
        (apo << 6) | (irqen << 5) | (enadc << 4) | (enldo << 3) | (enrefo << 2) | (enop << 1);
    writeRegister(0x00, data);
}

void HY3118::REG_1(InputChannel inn, InputChannel inp)
{
    uint8_t data = (inn << 3) | inp;
    writeRegister(0x01, data);
}

void HY3118::REG_2(ReferenceVoltageP vrps, ReferenceVoltageN vrns, DCoffset dcset)
{
    uint8_t data = (vrps << 6) | (vrns << 4) | (dcset);
    writeRegister(0x2, data);
}

void HY3118::REG_3(OscillatorSource osc, FullRange frb, PGA gain_pga, ADGN gain_adgn)
{
    uint8_t data = (osc << 6) | (frb << 5) | (gain_pga << 2) | (gain_adgn);
    writeRegister(0x3, data);
}

void HY3118::REG_4(LDOVoltage ldo, ReferenceVoltage refo, HighSpeed hs, ADCOutputRate osr)
{
    uint8_t data = (ldo << 6) | (refo << 5) | (hs << 4) | (osr << 1);
    writeRegister(0x4, data);
}

// get the tare offset (raw data value output without the scale "calFactor")
long HY3118::getTareOffset()
{
    return tareOffset;
}

// set new tare offset (raw data value input without the scale "calFactor")
void HY3118::setTareOffset(long offset)
{
    tareOffset = offset;
}

float HY3118::getSmoothedData() // return fresh data from the moving average dataset
{
    updateRawData();

    if (readDataTimeoutFlag == 0)
    {
        smoothedDataSum = smoothedDataSum - dataSampleSet[readIndex] + adcRawData;
        dataSampleSet[readIndex] = adcRawData;
        readIndex = (readIndex + 1) % SAMPLES;
        smoothedDataAvg = (smoothedDataSum / SAMPLES);

        return smoothedDataAvg;
    }
    else
    {
        printf("updateRawData() is timeout.\n");
        return 0;
    }
}

// set new calibration factor, raw data is divided by this value to convert to readable data
void HY3118::setCalFactor(float cal)
{
    calFactor = cal;
    calFactorRecip = 1 / calFactor;
}

// returns the current calibration factor
float HY3118::getCalFactor()
{
    return calFactor;
}

// zero the scale, wait for tare to finish (blocking)
void HY3118::tare()
{
    for (int i = 0; i < SAMPLES * 3; i++)
    {
        getSmoothedData();
    }

    setTareOffset((long)getSmoothedData());
    isTareDone = 1;
}

long HY3118::getRawData()
{
    return adcRawData;
}

float HY3118::getWeight(int sampleSize)
{
    float ret;
    for (int i = 0; i < sampleSize; i++)
    {
        ret = getSmoothedData();
    }
#if (HY3118_DBG == 1)
    printf("smoothed:%f, tareOffset:%f, calfactor:%f", ret, (float)tareOffset, calFactor);
#endif
    ret = (ret - (float)tareOffset) * calFactorRecip;
    ret = (ret < 0.0) ? 0.0 : ret;
    return ret;
}