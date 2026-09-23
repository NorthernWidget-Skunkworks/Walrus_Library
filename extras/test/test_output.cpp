// Output-regression test for Walrus_Library: compiles src/Walrus_I2C.cpp
// against the NW_Core stubs and prints getHeader()/getString()/getters for
// fixed register images. run.sh diffs the result against baseline.txt.
#include "Arduino.h"
#include "Wire.h"
TwoWire Wire;
#include "../../src/Walrus_I2C.cpp"

static uint8_t crc8(const uint8_t* d, uint8_t n) {           // CRC-8/SMBUS, as NW-Provision writes it
  uint8_t c = 0; for (uint8_t i = 0; i < n; i++) { c ^= d[i]; for (int b = 0; b < 8; b++) c = (c & 0x80) ? (c << 1) ^ 0x07 : (c << 1); }
  return c;
}

// Build a Schema 1 register image: Page 0 as NW-Provision writes it (with the
// firmware's patch at 0x0A), Page 1 with a complete reading (Walrus appendix:
// pressure int32 uBar at 0x28, MS5803 temperature int16 0.01 C at 0x2C,
// external temperature int16 0.01 C at 0x30).
static void loadImage(int32_t pressure, int16_t tMS5803, int16_t tExt, uint8_t fwPatch = 1, uint8_t schema = 0x01) {
  uint8_t* r = Wire.image; memset(r, 0, sizeof(Wire.image));
  r[0x00] = schema; memcpy(r + 0x01, "Walrus", 6);
  r[0x08] = 0; r[0x09] = 2; r[0x0A] = fwPatch;                       // HW 0.2, FW patch
  r[0x10] = 0x57; r[0x11] = 0x02; r[0x12] = 0; r[0x13] = 7; r[0x14] = 0; r[0x15] = 42;
  r[0x1D] = 0x4E; r[0x1E] = crc8(r, 0x1E); r[0x1F] = 0x57;
  r[0x20] = 0x01;                                                   // ready
  r[0x21] = 0x06;                                                   // both chips selected
  r[0x22] = 1; r[0x23] = 0;                                         // reading counter = 1
  r[0x26] = 0x00; r[0x27] = 0x00;
  for (int i = 0; i < 4; i++) r[0x28 + i] = (pressure >> (8 * i)) & 0xFF;
  r[0x2C] = tMS5803 & 0xFF; r[0x2D] = (tMS5803 >> 8) & 0xFF;
  r[0x30] = tExt & 0xFF;    r[0x31] = (tExt >> 8) & 0xFF;
}

// Emulate the Schema 1 firmware's response to a control write: a trigger
// completes a reading at once (counter +1, ready set, trigger and sleep bits
// cleared, fault byte cleared). A per-test hook can vary the data.
static std::function<void(TwoWire&)> onReading;
static void installFirmwareEmulation() {
  Wire.onWrite = [](TwoWire& w, uint8_t reg, uint8_t val) {
    if (reg != 0x21) return;
    w.image[0x27] = 0;
    if (!(val & 0x01)) return;
    w.image[0x21] = val & 0x7E;
    if (onReading) onReading(w);
    uint16_t c = w.image[0x22] | (w.image[0x23] << 8); c++;
    w.image[0x22] = c & 0xFF; w.image[0x23] = c >> 8;
    w.image[0x20] |= 0x01;
  };
}

static void report(const char* name, Walrus& s) {
  printf("[%s]\n", name);
  printf("header: %s\n", s.getHeader().c_str());
  printf("string: %s\n", s.getString().c_str());
  printf("getters: pressure=%.4f tExt=%.4f tMS5803=%.4f default=%.4f newData=%d\n",
         s.getPressure(), s.getTemperature(0), s.getTemperature(1), s.getTemperature(), s.newData());
}

int main() {
  Wire.deviceAddress = 0x57;
  installFirmwareEmulation();

  // 1. A complete reading: 1013.250 mBar, 21.37 C in the MS5803, 4.05 C in the water.
  loadImage(1013250, 2137, 405);
  { Walrus s; s.begin(); report("complete reading", s); }

  // 2. Negative temperatures and a depth reading (2.5 m of water over 1 bar).
  loadImage(1250000, -1234, -50);
  { Walrus s; s.begin(); report("negative temperatures", s); }

  // 3. Device absent: no acknowledge at the address.
  loadImage(1013250, 2137, 405); Wire.present = false;
  { Walrus s; s.begin(); report("device absent", s); }
  Wire.present = true;

  // 4. Device present but never ready (status bit 0 clear, counter frozen).
  loadImage(1013250, 2137, 405); Wire.image[0x20] = 0x00; Wire.onWrite = nullptr;
  { Walrus s; s.begin(); report("never ready", s); }
  installFirmwareEmulation();

  fprintf(stderr, "bus transactions total: %u\n", Wire.transactions);   // metric, not output
  return 0;
}
