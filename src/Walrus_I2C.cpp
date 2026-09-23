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

bool Walrus::updateMeasurements(uint8_t component)
{
    bool doMS = component & MS5803, doMCP = component & MCP9808;
    if(doMS) { _pressureReadings.reset(); _tempMS5803Readings.reset(); }
    if(doMCP) _tempExtReadings.reset();
    if(doMS && doMCP && _nPressureReadings <= 1 && _nTemperatureReadings <= 1) {
        //One reading of everything: both chips in one trigger, one 10-byte read.
        _dev.resetBatch();
        uint8_t d[10];
        if(_dev.takeReading(ALL) && _dev.readBytes(NW_REG_DATA, d, 10)) {
            readMS5803(d);
            readMCP9808(d + 8);
        }
    }
    else {
        //Per chip group: N readings each, appended to the arrays; a chip that
        //reports absent (no acknowledge / not initialised) stops its batch.
        if(doMS) {
            _dev.beginBatch(_nPressureReadings);
            for(uint16_t i = 0; i < _nPressureReadings; i++) {
                if(!updatePressure() && _dev.batchFaulted(MS5803)) break;
            }
        }
        if(doMCP) {
            _dev.beginBatch(_nTemperatureReadings);
            for(uint16_t i = 0; i < _nTemperatureReadings; i++) {
                if(!updateTemperature() && _dev.batchFaulted(MCP9808)) break;
            }
        }
    }
    summarise(component);
    bool ok = true;
    if(doMS) ok = ok && _pressureReadings.count() > 0;
    if(doMCP) ok = ok && _tempExtReadings.count() > 0;
    return ok;
}

bool Walrus::updatePressure()
{
    uint8_t d[6];
    if(!_dev.takeReading(MS5803) || !_dev.readBytes(PRES_REG, d, 6)) return false;
    return readMS5803(d);
}

bool Walrus::updateTemperature()
{
    uint8_t d[2];
    if(!_dev.takeReading(MCP9808) || !_dev.readBytes(TEMP_EXT, d, 2)) return false;
    return readMCP9808(d);
}

bool Walrus::readMS5803(uint8_t* d)
{
    if(_dev.faulted(0)) return false;           //MS5803: pressure int32 uBar, temperature int16 0.01 C
    int32_t p = (int32_t)((uint32_t)d[0] | ((uint32_t)d[1] << 8) | ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24));
    _pressureReadings.append(p);
    _tempMS5803Readings.append((int16_t)(d[4] | (d[5] << 8)));
    return true;
}

bool Walrus::readMCP9808(uint8_t* d)
{
    if(_dev.faulted(1)) return false;           //MCP9808: external temperature int16 0.01 C
    _tempExtReadings.append((int16_t)(d[0] | (d[1] << 8)));
    return true;
}

void Walrus::summarise(uint8_t component)
{
    //Means over the readings taken, scaled from the register units; NW_ERROR when none.
    if(component & MS5803) {
        _pressure   = _pressureReadings.count()   ? _pressureReadings.mean() / 1000.0   : NW_ERROR;
        _tempMS5803 = _tempMS5803Readings.count() ? _tempMS5803Readings.mean() / 100.0  : NW_ERROR;
    }
    if(component & MCP9808) {
        _tempExt = _tempExtReadings.count() ? _tempExtReadings.mean() / 100.0 : NW_ERROR;
    }
}

uint16_t Walrus::setPressureReadings(uint16_t n)
{
    _nPressureReadings = (n > WALRUS_PRESSURE_CAPACITY) ? WALRUS_PRESSURE_CAPACITY : n;
    return _nPressureReadings;
}

uint16_t Walrus::setTemperatureReadings(uint16_t n)
{
    _nTemperatureReadings = (n > WALRUS_TEMPERATURE_CAPACITY) ? WALRUS_TEMPERATURE_CAPACITY : n;
    return _nTemperatureReadings;
}

void     Walrus::setPressureStats(bool enable)    { _pressureStats = enable; }
void     Walrus::setTemperatureStats(bool enable) { _temperatureStats = enable; }
uint16_t Walrus::getPressureCount()               { return _pressureReadings.count(); }
uint16_t Walrus::getTemperatureCount()            { return _tempExtReadings.count(); }

//Statistics are computed from the arrays each call (NW_Readings), in the
//register units, then scaled: uBar -> mBar, 0.01 C -> C. NW_ERROR when empty.
static float scaled(float v, float divisor) { return (v == NW_ERROR) ? NW_ERROR : v / divisor; }
float Walrus::getPressureMean()   { return scaled(_pressureReadings.mean(),   1000.0); }
float Walrus::getPressureStd()    { return scaled(_pressureReadings.std(),    1000.0); }
float Walrus::getPressureSterr()  { return scaled(_pressureReadings.sterr(),  1000.0); }
float Walrus::getPressureMedian() { return scaled(_pressureReadings.median(), 1000.0); }
float Walrus::getTemperatureMean(uint8_t Location)   { return scaled(Location == 0 ? _tempExtReadings.mean()   : _tempMS5803Readings.mean(),   100.0); }
float Walrus::getTemperatureStd(uint8_t Location)    { return scaled(Location == 0 ? _tempExtReadings.std()    : _tempMS5803Readings.std(),    100.0); }
float Walrus::getTemperatureSterr(uint8_t Location)  { return scaled(Location == 0 ? _tempExtReadings.sterr()  : _tempMS5803Readings.sterr(),  100.0); }
float Walrus::getTemperatureMedian(uint8_t Location) { return scaled(Location == 0 ? _tempExtReadings.median() : _tempMS5803Readings.median(), 100.0); }

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
    String h = "Pressure [mBar],"; //return header string
    if(_pressureStats && _nPressureReadings > 1) h += "Pressure std [mBar],Pressure sterr [mBar],";
    h += "Temp DH [C],";
    if(_temperatureStats && _nTemperatureReadings > 1) h += "Temp DH std [C],Temp DH sterr [C],";
    h += "Temp DHt [C],";
    if(_pressureStats && _nPressureReadings > 1) h += "Temp DHt std [C],Temp DHt sterr [C],";
    return h;
}

String Walrus::getString()
{
    updateMeasurements();                           //NW_ERROR (-9999) where a reading failed
    String s = String(getPressure()) + ",";
    if(_pressureStats && _nPressureReadings > 1) s += String(getPressureStd()) + "," + String(getPressureSterr()) + ",";
    s += String(getTemperature(0)) + ",";
    if(_temperatureStats && _nTemperatureReadings > 1) s += String(getTemperatureStd(0)) + "," + String(getTemperatureSterr(0)) + ",";
    s += String(getTemperature(1)) + ",";
    if(_pressureStats && _nPressureReadings > 1) s += String(getTemperatureStd(1)) + "," + String(getTemperatureSterr(1)) + ",";
    return s;
}

//The reading interface: one reading per logReading(), printed as it is taken.
void Walrus::beginReadings(uint8_t component, uint16_t n)
{
    _component = component;
    if(component & MS5803) { _pressureReadings.reset(); _tempMS5803Readings.reset(); }
    if(component & MCP9808) _tempExtReadings.reset();
    _dev.beginBatch(n);
}

void Walrus::endReadings()
{
    //No cleanup required currently
}

size_t Walrus::printHeader(Print& out)
{
    size_t n = 0;
    if(_component & MS5803) n += out.print("Pressure [mBar],Temp DHt [C],");
    if(_component & MCP9808) n += out.print("Temp DH [C],");
    return n;
}

size_t Walrus::printReading(Print& out)
{
    size_t n = 0;
    if(_component & MS5803) { n += out.print(_pressure); n += out.print(','); n += out.print(_tempMS5803); n += out.print(','); }
    if(_component & MCP9808) { n += out.print(_tempExt); n += out.print(','); }
    return n;
}

size_t Walrus::logReading(Print& out)
{
    //One acquisition per chip group selected, then the values just taken.
    if(_component & MS5803) {
        uint8_t d[6];
        _pressure = _tempMS5803 = NW_ERROR;
        if(_dev.takeReading(MS5803) && _dev.readBytes(PRES_REG, d, 6) && readMS5803(d)) {
            _pressure = _pressureReadings.last() / 1000.0;
            _tempMS5803 = _tempMS5803Readings.last() / 100.0;
        }
    }
    if(_component & MCP9808) {
        uint8_t d[2];
        _tempExt = NW_ERROR;
        if(_dev.takeReading(MCP9808) && _dev.readBytes(TEMP_EXT, d, 2) && readMCP9808(d)) {
            _tempExt = _tempExtReadings.last() / 100.0;
        }
    }
    return printReading(out);
}
