#ifndef HY3118_H
#define HY3118_H

#include <Wire.h>
#include "config.h"

#define DATA_SET SAMPLES + IGN_HIGH_SAMPLE + IGN_LOW_SAMPLE // total samples in memory

#if (SAMPLES == 1)
#define DIVB 0
#elif (SAMPLES == 2)
#define DIVB 1
#elif (SAMPLES == 4)
#define DIVB 2
#elif (SAMPLES == 8)
#define DIVB 3
#elif (SAMPLES == 16)
#define DIVB 4
#elif (SAMPLES == 32)
#define DIVB 5
#elif (SAMPLES == 64)
#define DIVB 6
#elif (SAMPLES == 128)
#define DIVB 7
#endif

// REG 1
enum InputChannel
{
    AIN1 = 0b000,
    AIN2 = 0b001,
    AIN3 = 0b010,
    AIN4 = 0b011,
    REFO = 0b100,
    OPO = 0b101,
    VSSA_ = 0b110,
    VDD_DIV_10 = 0b111 // 0.1xVDD
};

// REG 2
enum ReferenceVoltageP
{
    Vrefp_unbuffered = 0b00,
    VDDA = 0b01,
    Vrefp_buffered = 0b10,
    V12p = 0b11
};
enum ReferenceVoltageN
{
    Vrefn_unbuffered = 0b00,
    VSSA = 0b01,
    Vrefn_buffered = 0b10,
    V12n = 0b11
};

enum DCoffset
{
    VREF_0 = 0b0000,
    VREF_1_8 = 0b0001,
    VREF_1_4 = 0b0010,
    VREF_3_8 = 0b0011,
    VREF_1_2 = 0b0100,
    VREF_5_8 = 0b0101,
    VREF_3_4 = 0b0110,
    VREF_7_8 = 0b0111,
    VREF_neg_1_8 = 0b1001,
    VREF_neg_1_4 = 0b1010,
    VREF_neg_3_8 = 0b1011,
    VREF_neg_1_2 = 0b1100,
    VREF_neg_5_8 = 0b1101,
    VREF_neg_3_4 = 0b1110,
    VREF_neg_7_8 = 0b1111
};

// REG 3
enum OscillatorSource
{
    Internal327KHz = 0b00,
    Internal1000KHz = 0b01,
    ExternalDiv15 = 0b10,
    ExternalDiv5 = 0b11
};

enum FullRange
{
    Full = 0b0,
    Half = 0b1
};

enum PGA
{
    Disable = 0b000,
    Gain8 = 0b001,
    Gain16 = 0b011,
    Gain32 = 0b111
};

enum ADGN
{
    Gain1 = 0b00,
    Gain2 = 0b01,
    Gain4 = 0b11
};

// REG 4

enum LDOVoltage
{
    ldo_3v3 = 0b00,
    ldo_3v0 = 0b01,
    ldo_2v7 = 0b10,
    ldo_2v4 = 0b11
};

enum ReferenceVoltage
{
    refo_1v2 = 0b0,
    refo_1v5 = 0b1
};

enum HighSpeed
{
    Slow_327kHz = 0b0,
    High_1000kHz = 0b1
};

enum ADCOutputRate
{
    Rate2560_7680 = 0b000,
    Rate1280_3840 = 0b001,
    Rate640_1920 = 0b010,
    Rate320_960 = 0b011,
    Rate160_480 = 0b100,
    Rate80_240 = 0b101,
    Rate40_120 = 0b110,
    Rate10_30 = 0b111
};

// ADC ready
enum AdcOutputStatus
{
    NotReady = 0b0,
    Ready = 0b1
};

enum ReferenceVoltagePolarity
{
    Positive = 0,
    Negative = 1
};

class HY3118
{
public:
    HY3118(uint8_t address);
    void begin();

    void REG_0(bool apo, bool irqen, bool enadc, bool enldo, bool enrefo, bool enop);
    // ADC1 暫存器操作
    void REG_1(InputChannel inn, InputChannel inp);
    // 設定正負參考電壓, DC 偏移電壓
    void REG_2(ReferenceVoltageP vrps, ReferenceVoltageN vrns, DCoffset dcset);
    // 設定振盪器、增益、參考電壓範圍
    void REG_3(OscillatorSource osc, FullRange frb, PGA gain_pga, ADGN gain_adgn);
    // 設定 LDO 電壓、參考電壓、採樣率
    void REG_4(LDOVoltage ldo, ReferenceVoltage refo, HighSpeed hs, ADCOutputRate osr);
    void updateData();
    void writeRegister(uint8_t reg, uint8_t data);
    uint8_t readRegister(uint8_t reg);
    long getTareOffset(void);
    void setTareOffset(long newoffset);
    float getData(void);
    void setCalFactor(float cal);
    float getCalFactor(void);
    void tare(void); // zero the scale, wait for tare to finnish (blocking)
    bool isDataReady(void);
    long getAdcData(void);

protected:
    long smoothedData(); // returns the smoothed data value calculated from the dataset

    uint8_t _address;
    float calFactor = 1.0;      // calibration factor as given in function setCalFactor(float cal)
    float calFactorRecip = 1.0; // reciprocal calibration factor (1/calFactor), the HX711 raw data is multiplied by this value
    long tareOffset = 0;
    int samplesInUse = SAMPLES;
    long lastSmoothedData = 0;
    uint8_t divBit = DIVB;
    volatile long dataSampleSet[DATA_SET + 1];
    bool tareTimeoutFlag = 0;
    unsigned int tareTimeOut = (SAMPLES + IGN_HIGH_SAMPLE + IGN_HIGH_SAMPLE) * 150; // tare timeout time in ms, no of samples * 150ms (10SPS + 50% margin)
    bool tareTimeoutDisable = 0;
    int readIndex = 0;
    bool isTareDone = 0;
    bool isAdcDataReady = 0;
    long adcData = 0;
};

#endif