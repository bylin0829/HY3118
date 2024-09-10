#include "Arduino.h"
#include "Wire.h"
#include "HY3118.h"

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
        return 0; // 讀取失敗
    }
}

void HY3118::updateData()
{
    Wire.beginTransmission(_address);
    Wire.write(0x5);
    Wire.endTransmission(true);

    Wire.requestFrom(_address, (size_t)3);
    uint32_t adcVal = 0;
    int i = 2;
    while (Wire.available() > 0)
    {
        uint32_t temp = Wire.read();
        adcVal |= (temp << (i * 8));
        i--;
    }

    isAdcDataReady = 0;
    if (adcVal & 0x01)
    {
        adcData = adcVal >> 1;
        isAdcDataReady = 1;
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
    writeRegister(0x3, data); // ADC3 暫存器位址
}

void HY3118::REG_4(LDOVoltage ldo, ReferenceVoltage refo, HighSpeed hs, ADCOutputRate osr)
{
    uint8_t data = (ldo << 6) | (refo << 5) | (hs << 4) | (osr << 1);
    writeRegister(0x4, data); // ADC4 暫存器位址
}

// get the tare offset (raw data value output without the scale "calFactor")
long HY3118::getTareOffset()
{
    return tareOffset;
}

// set new tare offset (raw data value input without the scale "calFactor")
void HY3118::setTareOffset(long newoffset)
{
    tareOffset = newoffset;
}

float HY3118::getData() // return fresh data from the moving average dataset
{
    long data = 0;
    lastSmoothedData = smoothedData();
    data = lastSmoothedData - tareOffset;
    float x = (float)data * calFactorRecip;
    return x;
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

// zero the scale, wait for tare to finnish (blocking)
void HY3118::tare()
{
    float ret = 0;
    for (int i = 0; i < 30;)
    {
        updateData();
        if (isDataReady())
        {
            ret += getAdcData();
            i++;
        }
    }
    setTareOffset((long)(ret / 30));
    isTareDone = 1;
}

bool HY3118::isDataReady()
{
    return isAdcDataReady;
}
long HY3118::getAdcData()
{
    return adcData - tareOffset;
}