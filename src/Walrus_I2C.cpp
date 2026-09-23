/******************************************************************************
Walrus.cpp
Library for Walrus pressure and temperature sensor, made by Northern Widget LLC.
Based off of the TP-Downhole
Bobby Schulz @ Northern Widget LLC
5/9/2018
Hardware info located at:
https://github.com/NorthernWidget-Skunkworks/Project-Walrus

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h> // Wire library is used for I2C
#include "Walrus_I2C.h"

Walrus::Walrus()
{
}

bool Walrus::begin(uint8_t Address_)
{
    //Page 0 gates: Schema 1, the name "Walrus", firmware patch >= WALRUS_FW_MIN_PATCH.
    return _dev.begin(Address_, "Walrus", WALRUS_FW_MIN_PATCH);
}

bool Walrus::updateMeasurements()
{
    _pressure = _tempExt = _tempMS5803 = NW_ERROR;
    if(!_dev.takeReading(0x03)) return false;       //both chips: bit 0 MS5803, bit 1 MCP9808
    //Block 1 and Block 2 are consecutive (0x28-0x31): one read.
    uint8_t d[10];
    if(!_dev.readBytes(NW_REG_DATA, d, 10)) return false;
    if(!_dev.faulted(0)) {                          //MS5803: pressure int32 uBar, temperature int16 0.01 C
        int32_t p = (int32_t)((uint32_t)d[0] | ((uint32_t)d[1] << 8) | ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24));
        _pressure = float(p) / 1000.0;              //uBar -> mBar
        _tempMS5803 = float((int16_t)(d[4] | (d[5] << 8))) / 100.0;
    }
    if(!_dev.faulted(1)) {                          //MCP9808: external temperature int16 0.01 C
        _tempExt = float((int16_t)(d[8] | (d[9] << 8))) / 100.0;
    }
    return !_dev.anyFault();
}

float Walrus::getTemperature(uint8_t Location) //Returns temp in C from either subsensor
{
    return (Location == 0) ? _tempExt : _tempMS5803;
}

float Walrus::getTemperature() //By default get thermistor temp value
{
    return getTemperature(1);
}

float Walrus::getPressure()
{
    return _pressure;
}

bool Walrus::newData()  //Checks for updated/valid data
{
    return _dev.ready();
}

bool    Walrus::ready()            { return _dev.ready(); }
bool    Walrus::newReading()       { return _dev.newReading(); }
bool    Walrus::requestReading()   { return _dev.requestReading(0x03); }
bool    Walrus::faulted(uint8_t chip) { return _dev.faulted(chip); }
bool    Walrus::anyFault()         { return _dev.anyFault(); }
uint8_t Walrus::faultChip()        { return _dev.faultChip(); }
uint8_t Walrus::faultKind()        { return _dev.faultKind(); }
String  Walrus::beginFailure()     { return _dev.beginFailure(); }
uint8_t Walrus::getHardwareMajor() { return _dev.hardwareMajor(); }
uint8_t Walrus::getHardwareMinor() { return _dev.hardwareMinor(); }
uint8_t Walrus::getFirmwareVersion() { return _dev.firmwareVersion(); }

size_t Walrus::printFault(Print& out)
{
    //The chip names are Walrus's own; the kind names are universal (NW_Fault).
    static const char* const chips[] = {"MS5803", "MCP9808"};
    uint8_t chip = faultChip(), kind = faultKind();
    if(kind == 0) return out.print("none");
    size_t n = 0;
    if(chip == 7) n += out.print("unit");
    else if(chip < 2) n += out.print(chips[chip]);
    else { n += out.print("chip "); n += out.print(chip); }
    n += out.print(": ");
    return n + _dev.fault().printKind(out);
}

String Walrus::faultNote()
{
    //One word for a data-table note: the chip, then the kind ("MS5803NoACK").
    static const char* const chips[] = {"MS5803", "MCP9808"};
    uint8_t chip = faultChip();
    String w;
    if(chip == 7) w = F("Unit");
    else if(chip < 2) w = chips[chip];
    else { w = F("Chip"); w += String(chip); }
    w += _dev.fault().kindWord();
    return w;
}

String Walrus::getHeader()
{
    return "Pressure [mBar],Temp DH [C],Temp DHt [C],"; //return header string
}

String Walrus::getString()
{
    updateMeasurements();                           //NW_ERROR (-9999) where a reading failed
    return String(getPressure()) + "," + String(getTemperature(0)) + "," \
                                 + String(getTemperature(1)) + ",";
}
