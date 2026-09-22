# Walrus_Library

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4572367.svg)](https://doi.org/10.5281/zenodo.4572367)

Library for the Northern Widget [Walrus](https://github.com/NorthernWidget-Skunkworks/Project-Walrus) submersible pressure and temperature sensor. Based off of the TP-Downhole.

The Walrus is an encapsulated submersible pressure and temperature sensor intended for water-level or barometric monitoring.

**Installation:** included in [NorthernWidget-libraries](https://github.com/NorthernWidget/NorthernWidget-libraries).

```cpp
#include <Walrus_I2C.h>

Walrus sensor;

void setup() {
    Serial.begin(9600);
    sensor.begin();
    Serial.println(sensor.getHeader());
}

void loop() {
    Serial.println(sensor.getString());
    delay(1000);
}
```

**Full API reference:** https://docs.northernwidget.com/Walrus_Library/
