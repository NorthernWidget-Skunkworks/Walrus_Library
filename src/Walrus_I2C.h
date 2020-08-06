/******************************************************************************
TP_Downhole.h
Library for TP-Downhole pressure and tempurature sensor, made by Northern Widget LLC.
Bobby Schulz @ Northern Widget LLC
5/9/2018
Hardware info located at: https://github.com/NorthernWidget/TP-DownHole

The TP-Downhole is a small form factor tempreture and pressure sensor which uses a combination of an 
MS5803 pressure sensor and an MCP3421 ADC in combination with a thermistor for high accuracy
tempreture measurment.

"Size matter not. Look at me. Judge me by my size do you? And well you should not. For my ally is the
Force, and a powerful ally it is"
-Yoda

Distributed as-is; no warranty is given.
******************************************************************************/

#ifndef Walrus_I2C_h
#define Walrus_I2C_h

#include <Arduino.h>

// #define TEMP_REG_0 0x06 //Location of external temp val on Walrus 
#define TEMP_REG_0 0x06 //Location of external temp val on Walrus 
// #define TEMP_REG_1 0x09 //Location of MS5803 temp val on Walrus
#define TEMP_REG_1 0x08 //Location of MS5803 temp val on Walrus
// #define TEMP_OFFSET 0x03 //Define length (offset) of tempurate vals in bytes
#define TEMP_OFFSET 0x02 //Define length (offset) of tempurate vals in bytes
// #define PRES_REG 0x03 //Location of pressure register on Walrus
#define PRES_REG 0x02 //Location of pressure register on Walrus

#define ADR_DEFAULT 0x4D //Define default address
class Walrus
{
public:
    Walrus();  // Constructor
    uint8_t begin(uint8_t Address_ = ADR_DEFAULT); // Collect address value, use default

    // Return calculated temperature from sensor
    float getTemperature(uint8_t Location);
    float getTemperature();
    // Return calculated pressure from sensor
    float getPressure();
    String GetHeader();
    String GetString();

private:
    uint8_t ADR = ADR_DEFAULT; //Default address
};

#endif
