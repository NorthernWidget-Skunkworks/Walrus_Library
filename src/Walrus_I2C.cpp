/******************************************************************************
Walrus.cpp
Library for TP-Downhole pressure and tempurature sensor, made by Northern Widget LLC.
Bobby Schulz @ Northern Widget LLC
5/9/2018
Hardware info located at: https://github.com/NorthernWidget/TP-DownHole

The TP-Downhole is a small form factor tempreture and pressure sensor which uses a combination of an 
MS5803 pressure sensor and an MCP3421 ADC in combination with a thermistor for high accuracy
tempreture measurment.

"Size matters not. Look at me. Judge me by my size do you? And well you should not. For my ally is the
Force, and a powerful ally it is"
-Yoda

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h> // Wire library is used for I2C
#include "Walrus_I2C.h"

Walrus::Walrus()
// Base library type I2C
{
}


uint8_t Walrus::begin(uint8_t Address_)
{
    Wire.begin(); //Initialize Wire instance 
    ADR = Address_; //Copy address to local
}

// float Walrus::getTemperature(uint8_t Location) //Returns temp in C from either subsensor
// {
//     long TempData[TEMP_OFFSET] = {0}; //Make temp data storage location
//     uint8_t Offset = 0; //Default to TEMP_REG_0
//     int Error = 0; //Error used for testing transmission
//     if(Location == 1) Offset = 0x03;
//     // Wire.beginTransmission(ADR);
//     // Wire.write(TEMP_REG_0 + Offset); 
//     // Error = Wire.endTransmission();

//     // Wire.requestFrom(ADR, 3); //Read 3 bytes from device
//     for(int i = 0; i < TEMP_OFFSET; i++) { //Increment over read
//         Wire.beginTransmission(ADR);
//         Wire.write(TEMP_REG_0 + Offset + i); 
//         Error = Wire.endTransmission();
//         Wire.requestFrom((int)ADR, 1); //Cast ADR to match function
//         TempData[i] = Wire.read(); //Read in data
//     }
//     // Serial.print("\n\n"); //DEBUG!
//     // Serial.println(TEMP_REG_0 + Offset, HEX);
//     // Serial.println(TempData[2], HEX);
//     // Serial.println(TempData[1], HEX);
//     // Serial.println(TempData[0], HEX);
//     if(Error == 0) return float(long((TempData[2] << 16) | (TempData[1] << 8) | (TempData[0])))/1000.0; //If no error, return concatonated, scaled value
//     else return -9999.0; //Else return error condition 
// }

float Walrus::getTemperature(uint8_t Location) //Returns temp in C from either subsensor
{
    long TempData[TEMP_OFFSET] = {0}; //Make temp data storage location
    uint8_t Offset = 0; //Default to TEMP_REG_0
    int Error = 0; //Error used for testing transmission
    if(Location == 1) Offset = 0x02;
    // Wire.beginTransmission(ADR);
    // Wire.write(TEMP_REG_0 + Offset); 
    // Error = Wire.endTransmission();

    // Wire.requestFrom(ADR, 3); //Read 3 bytes from device
    for(int i = 0; i < TEMP_OFFSET; i++) { //Increment over read
        Wire.beginTransmission(ADR);
        Wire.write(TEMP_REG_0 + Offset + i); 
        Error = Wire.endTransmission();
        Wire.requestFrom((int)ADR, 1); //Cast ADR to match function
        TempData[i] = Wire.read(); //Read in data
    }
    // Serial.print("\n\n"); //DEBUG!
    // Serial.println(TEMP_REG_0 + Offset, HEX);
    // Serial.println(TempData[2], HEX);
    // Serial.println(TempData[1], HEX);
    // Serial.println(TempData[0], HEX);
    if(Error == 0) return float(long((TempData[1] << 8) | (TempData[0])))/100.0; //If no error, return concatonated, scaled value
    else return -9999.0; //Else return error condition 
}


float Walrus::getTemperature() //By default get thermistor temp value
{
    return getTemperature(1); 
}


// float Walrus::getPressure()
// // Return a pressure reading units mBar.
// {
//     long TempData[3] = {0}; //Make temp data storage location
//     int Error = 0; //Error used for testing transmission
//     // Wire.beginTransmission(ADR);  //Command for data
//     // Wire.write(PRES_REG); 
//     // Error = Wire.endTransmission();

//     for(int i = 0; i < 3; i++) { //Increment over read
//         Wire.beginTransmission(ADR);
//         Wire.write(PRES_REG + i); 
//         Error = Wire.endTransmission();
//         Wire.requestFrom((int)ADR, 1); //Cast ADR to match function
//         TempData[i] = Wire.read(); //Read in data
//     }

//     if(Error == 0) return float(long((TempData[2] << 16) | (TempData[1] << 8) | (TempData[0])))/1000.0; //If no error, return concatonated, scaled value
//     else return -9999.0; //Else return error condition 
// }

float Walrus::getPressure()
// Return a pressure reading units mBar.
{
    long TempData[4] = {0}; //Make temp data storage location
    int Error = 0; //Error used for testing transmission
    // Wire.beginTransmission(ADR);  //Command for data
    // Wire.write(PRES_REG); 
    // Error = Wire.endTransmission();

    for(int i = 0; i < 4; i++) { //Increment over read
        Wire.beginTransmission(ADR);
        Wire.write(PRES_REG + i); 
        Error = Wire.endTransmission();
        Wire.requestFrom((int)ADR, 1); //Cast ADR to match function
        TempData[i] = Wire.read(); //Read in data
    }

    long PresVal = 0;
    for(int i = 0; i < 4; i++) {
        PresVal = PresVal | (TempData[i] << 8*i);
    }

    if(Error == 0) return float(PresVal)/1000.0; //If no error, return concatonated, scaled value
    else return -9999.0; //Else return error condition 
}

String Walrus::GetHeader() 
{
    return "Pressure [mBar], Temp DH [C], Temp DHt [C], "; //return header string
}

String Walrus::GetString()
{
    return String(getPressure()) + "," + String(getTemperature(0)) + "," + String(getTemperature(1)) + ","; //Return data string
}
