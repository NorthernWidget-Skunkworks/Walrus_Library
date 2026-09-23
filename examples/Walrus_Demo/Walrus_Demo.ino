// Walrus_Demo: one row per second from a Walrus pressure and temperature
// sensor over I2C. Header once, then getString() takes a reading each loop.
#include <Walrus_I2C.h>

Walrus sensor;

void setup() {
  Serial.begin(9600);
  if (!sensor.begin()) {
    Serial.print("Walrus not found: ");
    Serial.println(sensor.beginFailure());  // NoACK, NotSchema1, WrongName, OldFirmware
  }
  Serial.println(sensor.getHeader());
}

void loop() {
  Serial.println(sensor.getString());  // -9999.00 where a reading failed
  if (sensor.anyFault()) {
    sensor.printReport(Serial);  // e.g. "MS5803: no acknowledge"
    Serial.println();
  }
  delay(1000);
}
