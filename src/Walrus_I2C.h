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
#include <NW_Core.h>   // NW_Core: NW_Device (Schema 1 protocol), NW_Fault

/// Lowest firmware patch (Page 0 byte 0x0A) this library accepts: patch 1
/// brought the Block 0 handshake (trigger, reading counter, faults).
#define WALRUS_FW_MIN_PATCH 1

#define PRES_REG    0x28  // Schema 1 Page 1 Block 1: pressure, int32, µBar
#define TEMP_MS5803 0x2C  // Schema 1 Page 1 Block 1: MS5803 temperature, int16, 0.01 °C
#define TEMP_EXT    0x30  // Schema 1 Page 1 Block 2: external temperature (MCP9808), int16, 0.01 °C

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
        /** @brief Default I2C address: NW-Device-Specification Schema 1 'W' (0x57). */
        static constexpr uint8_t DEFAULT_ADDRESS = 0x57;
        /**
         * @brief Instantiate Walrus object
         */
        Walrus();  // Constructor
        /**
         * @brief Begin communications with the Walrus using a prescribed
         * address.
         * @details Refuses the device unless Page 0 says Schema 1, the name
         * "Walrus", and a firmware patch of at least WALRUS_FW_MIN_PATCH;
         * beginFailure() says which gate refused.
         * @param Address_: I2C address of Walrus
         * @return true if the device answered and passed the three gates
         */
        bool begin(uint8_t Address_ = DEFAULT_ADDRESS);
        /**
         * @brief Take one reading of both chips and store it for the getters.
         * @details Triggers the device, waits for its reading counter to
         * advance, then reads pressure and both temperatures in one
         * transaction. A chip the device reports faulted leaves its values
         * at NW_ERROR (-9999).
         * @return true if the device answered and no chip faulted
         */
        bool updateMeasurements();
        /**
         * @brief Return calculated temperature from Walrus.
         * @details This calculated temperature can be from either
         * the MS5803 sensor, which primarily measures pressure, or from the
         * dedicated MCP9808 sensor. Values are those stored by the last
         * updateMeasurements() (NW_ERROR before the first).
         *
         * @param Location
         * 0: Read dedicated temperature sensor.
         * 1: Read temperature sensor within the MS5803.
         */
        float getTemperature(uint8_t Location);
        /**
         * @brief Return the MS5803 temperature (getTemperature(1)).
         */
        float getTemperature();
        /**
         * @brief Return calculated pressure from sensor [mBar].
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
         * @brief Take a reading (updateMeasurements()) and return it as a string
         * @details String(getPressure()) + "," + String(getTemperature(0))
         + "," + String(getTemperature(1)) + ",";
         */
        String getString();
        /**
        * @brief Checks for updated data. Returns `true` if the device's ready
        * bit is set; otherwise returns `false`.
        * @deprecated Use ready(); the handshake is newReading() after requestReading().
        */
        bool newData();

        // --- Handshake (NW-Device-Specification Block 0) ---
        /** @brief Status ready bit: the data registers hold a complete reading. */
        bool ready();
        /** @brief The reading counter has advanced since the last request. */
        bool newReading();
        /** @brief Trigger a reading of both chips without waiting for it. */
        bool requestReading();

        // --- Faults (status byte, live; fault byte, latched) ---
        /** @brief True if the given chip (0 = MS5803, 1 = MCP9808) was faulted in the last reading. */
        bool faulted(uint8_t chip);
        /** @brief True if any chip was faulted in the last reading (status pan-fault bit). */
        bool anyFault();
        /** @brief Chip index of the latched fault (0 MS5803, 1 MCP9808, 7 the unit); meaningful when faultKind() != 0. */
        uint8_t faultChip();
        /** @brief Kind of the latched fault, per the spec's table (1 no acknowledge, 6 reset since configured, ...). */
        uint8_t faultKind();
        /** @brief Print the latched fault as text, e.g. "MS5803: no acknowledge"; "none" when there is no fault. */
        size_t printFault(Print& out);
        /** @brief The latched fault as one word for a note column: "MS5803NoACK", "UnitReset"; "UnitNone" when none. */
        String faultNote();
        /** @brief Why the last begin() refused, as one word: "NoACK", "NotSchema1", "WrongName", "OldFirmware"; "None" after success. */
        String beginFailure();
        uint8_t getHardwareMajor();
        uint8_t getHardwareMinor();
        uint8_t getFirmwareVersion();
    private:
        NW_Device _dev;
        float _pressure = NW_ERROR;   //Stored by updateMeasurements() [mBar]
        float _tempExt = NW_ERROR;    //MCP9808 [C]
        float _tempMS5803 = NW_ERROR; //MS5803 [C]
};

/** @deprecated Use Walrus::DEFAULT_ADDRESS. Every NW library defined this same macro
 *  with a different value, so a sketch including two of them got the last one.
 *  Removed at the next major version. */
#define ADR_DEFAULT Walrus::DEFAULT_ADDRESS

#endif
