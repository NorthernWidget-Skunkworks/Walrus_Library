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

#ifndef Walrus_I2C_h
#define Walrus_I2C_h

#include <Arduino.h>

#define PRES_REG    0x22  // Schema 1 Page 1: pressure, int32, µBar
#define TEMP_MS5803 0x26  // Schema 1 Page 1: MS5803 temperature, int16, 0.01 °C
#define TEMP_EXT    0x28  // Schema 1 Page 1: external temperature (MCP9808), int16, 0.01 °C

#define ADR_DEFAULT 0x4D //Define default address

/**
 * @class Walrus: .
 * @brief Class to interface with the Walrus submersible temperature and
 * pressure sensor
 * @details The Walrus is an encapsulated submersible
 * pressure and temperature sensor intended for water-level or barometric
 * monitoring.
 *
 * \verbatim [![DOI](https://zenodo.org/badge/219609527.svg)](https://zenodo.org/badge/latestdoi/219609527) \endverbatim
 */
class Walrus
{
    public:

        /**
         * @brief Instantiate Walrus object
         */
        Walrus();  // Constructor

        /**
         * @brief Begin communications with the Walrus using a prescribed
         * address.
         * @param Address_: I2C address of Walrus
         */
        uint8_t begin(uint8_t Address_ = ADR_DEFAULT);

        /**
         * @brief Return calculated temperature from Walrus.
         * @details This calculated temperature can be from either
         * the MS5803 sensor, which primarily measures pressure, or from the
         *
         * @param Location
         * TEMP_REG_0: Read dedicated temperature sensor.
         * TEMP_REG_1: Read temperature sensor within the MS5803.
         * APPEARS TO JUST WORK WITH TEMP_REG_0 FOR NOW
         */
        float getTemperature(uint8_t Location);

        /**
         * @brief Return calculated temperature from sensor, using a predefined
         * offset.
         * WHAT DOES THIS OFFSET DO?
         */
        float getTemperature();

        /**
         * @brief Return calculated pressure from sensor.
         * @details This is the MS5803 sensor, which can come in a variety
         * of different pressure ranges and sensitivities.
         */
        float getPressure();

        /**
         * @brief Return header
         * @details "Pressure [mBar],Temp DH [C],Temp DHt [C],"
         */
        String getHeader();

        /**
         * @brief Return measurement data as a string
         * @details String(getPressure()) + "," + String(getTemperature(0))
         + "," + String(getTemperature(1)) + ",";
         */
        String getString();

        /**
        * @brief Checks for updated data. Returns `true` if new data are available;
        * otherwise returns `false`
        * @param Block: if `true`, wait for data to be returned. Defaults to
        * `false`.
        */
        bool newData();

    private:
        uint8_t ADR = ADR_DEFAULT; //Default address
        unsigned long timeoutGlobal = 1000; //Timeout value, ms //FIX??
};

#endif
