#ifndef HY3118_H
#define HY3118_H

#include <Wire.h>

#define SAMPLES 16 // default value: 16

// REG 0
enum apo
{
    APO_DISABLE,
    APO_ENABLE
};
enum irqen
{
    IRQEN_DISABLE,
    IRQEN_ENABLE
};
enum enadc
{
    ENADC_DISABLE,
    ENADC_ENABLE
};
enum enldo
{
    ENLDO_DISABLE,
    ENLDO_ENABLE
};
enum enrefo
{
    ENREFO_DISABLE,
    ENREFO_ENABLE
};
enum enop
{
    ENOP_DISABLE,
    ENOP_ENABLE
};
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
    void writeRegister(uint8_t reg, uint8_t data);
    uint8_t readRegister(uint8_t reg);
    void setTareOffset(long offset);
    long getTareOffset(void);
    void setCalFactor(float cal);
    float getCalFactor(void);
    void tare(void);             // zero the scale, wait for tare to finnish (blocking)
    float getSmoothedData(void); // returns the smoothed data value calculated from the dataset
    long getRawData(void);
    float getWeight(int sampleSize);

protected:
    void updateRawData();

    uint8_t _address;
    uint16_t readIndex = 0;
    uint32_t smoothedDataAvg = 0;
    volatile long dataSampleSet[SAMPLES];
    float calFactor = 1.0;      // calibration factor as given in function setCalFactor(float cal)
    float calFactorRecip = 1.0; // reciprocal calibration factor (1/calFactor), the HX711 raw data is multiplied by this value
    long tareOffset = 0;
    long lastSmoothedData = 0;
    long smoothedDataSum = 0;
    long adcRawData = 0;
    bool tareTimeoutFlag = 0;
    bool tareTimeoutDisable = 0;
    bool isTareDone = 0;
    bool isRawDataReady = 0;
    bool readDataTimeoutFlag = 0;
};

#endif