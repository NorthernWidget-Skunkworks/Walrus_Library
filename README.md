# Walrus_Library

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4572367.svg)](https://doi.org/10.5281/zenodo.4572367)

Library for the Northern Widget [Walrus](https://github.com/NorthernWidget/Project-Walrus) submersible pressure and temperature sensor. Based off of the TP-Downhole.

The Walrus is an encapsulated submersible pressure and temperature sensor intended for water-level or barometric monitoring.

**Installation:** included in [NorthernWidget-libraries](https://github.com/NorthernWidget/NorthernWidget-libraries).

```cpp
#include <Walrus_I2C.h>

Walrus sensor;

void setup() {
    Serial.begin(9600);
    if (!sensor.begin()) Serial.println(sensor.beginFailure());  // NoACK, NotSchema1, WrongName, OldFirmware
    Serial.println(sensor.getHeader());
}

void loop() {
    Serial.println(sensor.getString());
    delay(1000);
}
```

`getString()` triggers one reading of both chips through the [NW-Device-Specification](https://github.com/NorthernWidget/NW-Device-Specification) handshake (`updateMeasurements()`), waits for the device's reading counter, and prints pressure [mBar], the MCP9808 water temperature and the MS5803 temperature [°C], with `-9999.00` where a reading failed. The getters `getPressure()`, `getTemperature(0)` (MCP9808) and `getTemperature(1)` (MS5803) return the stored reading. `setPressureReadings(n)` and `setTemperatureReadings(n)` set how many readings of the MS5803 (pressure and its temperature) and of the MCP9808 (water temperature) each `getString()` takes (clamped to `WALRUS_PRESSURE_CAPACITY` and `WALRUS_TEMPERATURE_CAPACITY`, default 16; override before the include); the values printed are then the means, and `getPressureMean()`, `getPressureStd()`, `getPressureSterr()`, `getPressureMedian()`, the same for `getTemperature…(0)` and `(1)`, plus `getPressureCount()` and `getTemperatureCount()`, read the stored readings. With `setPressureStats(true)` or `setTemperatureStats(true)` the std and sterr columns join `getHeader()` and `getString()`. `updateMeasurements(Walrus::MS5803)` or `(Walrus::MCP9808)` reads one chip group alone. For one row per reading to a file, `beginReadings(component, n)`, `printHeader(out)`, `logReading(out)` n times, `endReadings()`, to any `Print` (an SdFat `File`, `Serial`).

Faults, for sketches that want them: `faulted(chip)`, `anyFault()`, `faultChip()`, `faultKind()`, `printFault(Serial)`, `faultNote()` (one word, e.g. `MS5803NoACK`, for a logger's note column). `begin()` refuses a device that is not Schema 1, not a Walrus, or below firmware patch `WALRUS_FW_MIN_PATCH`; `beginFailure()` says which. Requires the [NW_Core](https://github.com/NorthernWidget/NW_Core) library and Walrus firmware patch 1 or later.

**Full API reference:** https://docs.northernwidget.com/Walrus_Library/
