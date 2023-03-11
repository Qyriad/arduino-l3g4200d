#include "L3G4200D_U.h"

void L3G4200D_Unified::debugLog(const char str[]) {
  if (_debugLoggingEnabled) {
    Serial.print("[");
    Serial.print(_sensorId);
    Serial.print("]: ");
    Serial.print(str);
  }
}

void L3G4200D_Unified::debugLog(int val) {
  if (_debugLoggingEnabled) {
    Serial.print("[");
    Serial.print(_sensorId);
    Serial.print("]: ");
    Serial.print(val);
  }
}

void L3G4200D_Unified::debugAppend(const char str[]) {
  if (_debugLoggingEnabled) {
    Serial.print(str);
  }
}

void L3G4200D_Unified::debugAppend(int val) {
  if (_debugLoggingEnabled) {
    Serial.print(val);
  }
}

L3G4200D_Unified::L3G4200D_Unified(int32_t sensorId) {
  _sensorId = sensorId;
  _autoRangeEnabled = false;
  _debugLoggingEnabled = false;
}

bool L3G4200D_Unified::begin(int spiChipSelect, gyroRange_t range,
                             SPIClass &spi) {

  // Store the Chip Select we're using, set it as an output pin, and leave it
  // HIGH, as SPI CS is active LOW, and we don't want the gyroscope enabled yet.
  _spiCS = spiChipSelect;
  pinMode(_spiCS, OUTPUT);
  digitalWrite(_spiCS, HIGH);

  _range = range;

  // Store the SPI interface we're using...
  _spi = &spi;

  // .. and start it up.
  _spi->begin();

  SPISettings settings = SPISettings(5 * 1000 * 1000, MSBFIRST, SPI_MODE3);
  _spiSettings = settings;

  // Check that the chip ID is what we expect: 0b11010011, or 0xd3 in hex, and
  // 211 in decimal.
  beginTransaction();
  uint8_t chipId = spiReadReg(REG_WHO_AM_I);
  endTransaction();

  if (chipId == 0) {
    debugLog("We tried to read the L3G4200 gyroscope chip ID, but got all "
             "logic LOWs (0s) in response.\n");
    debugLog("Check that all your wires are connected properly?\n");
    return false;
  } else if (chipId == 0xFF) {
    debugLog("We tried to read the L3G4200 gyroscope chip ID, but got all "
             "logic HIGHs (1s) in response.\n");
    debugLog("Check that all your wires are connected properly?\n");
    return false;
  } else if (chipId != L3G4200D_CHIP_ID) {
    debugLog("We tried to read the L3G4200 gyroscope chip ID expecting ");
    debugLog(L3G4200D_CHIP_ID);
    debugLog(", but got ");
    debugLog(chipId);
    debugLog("\n");
    debugLog("Perhaps you have the wrong chip select connected or you're "
             "connected to a different part?\n");
    return false;
  }

  // Use a medium data rate and cutoff for the user, power on the gyroscope,
  // and enable all three axes.
  spiWriteReg(REG_CTRL_1, CTRL1_RATE_400HZ_CUTOFF_25HZ | CTRL1_XYZ);

  spiWriteReg(REG_CTRL_3, CTRL3_DRIVE_HIGH_AND_LOW);

  // Ask the gyroscope not to update the high byte and low byte of a sample
  // between reads, use the low byte at the lower address (as is the default),
  // and use the gyroscope range the user asked for.
  spiWriteReg(REG_CTRL_4, CTRL4_UPDATE_MSB_AND_LSB_TOGETHER |
                              CTRL4_LSB_AT_LOWER_ADDRESS | range);

  spiWriteReg(REG_CTRL_5, CTRL5_NO_FILTERING);

  return true;
}

void L3G4200D_Unified::enableAutoRange(bool enabled) {
  _autoRangeEnabled = enabled;
}

void L3G4200D_Unified::enableDebugLogging(bool enabled) {
  _debugLoggingEnabled = enabled;
}

bool L3G4200D_Unified::getEvent(sensors_event_t *event) {

  rawGyroSample sample = rawXYZ();

  // If we're supposed to be automatically changing the range, check if we're
  // saturating the sensor at the current range.
  if (_autoRangeEnabled) {

    // Give it a little bit of lee-way, in case it doesn't hit exactly 32767.
    const int16_t SATURATED_SAMPLE_VALUE = INT16_MAX - 10;
    if (abs(sample.x) >= SATURATED_SAMPLE_VALUE ||
        abs(sample.y) >= SATURATED_SAMPLE_VALUE ||
        abs(sample.z) >= SATURATED_SAMPLE_VALUE) {

      // Bump the range if we can, and re-read the sample.
      switch (_range) {
      // Intentional fallthrough.
      default:
      case GYRO_RANGE_4_DOT_36_RAD_PER_SEC:
        setRange(GYRO_RANGE_8_DOT_73_RAD_PER_SEC);
        sample = rawXYZ();
        break;

      case GYRO_RANGE_8_DOT_73_RAD_PER_SEC:
        setRange(GYRO_RANGE_34_DOT_91_RAD_PER_SEC);
        sample = rawXYZ();
        break;

      case GYRO_RANGE_34_DOT_91_RAD_PER_SEC:
        // We're already at maximum range; nothing to do here.
        break;
      }
    }
  }

  debugLog("Raw X, Y, Z samples: ");
  debugAppend(sample.x);
  debugAppend(", ");
  debugAppend(sample.y);
  debugAppend(", ");
  debugAppend(sample.z);
  debugAppend("\n");

  event->gyro.x = sampleToRad(sample.x);
  event->gyro.y = sampleToRad(sample.y);
  event->gyro.z = sampleToRad(sample.z);

  return true;
}

void L3G4200D_Unified::getSensor(sensor_t *sensor) {
  // Clear out the sensor data.
  memset(sensor, 0, sizeof(sensor_t));

  strncpy(sensor->name, "L3G4200D", sizeof(sensor->name) - 1);
  sensor->version = 1;
  sensor->sensor_id = _sensorId;
  sensor->type = SENSOR_TYPE_GYROSCOPE;

  // The maximum range this gyroscope supports.
  sensor->max_value = 31.94f;

  sensor->min_value = 0;

  // This is *U*INT16_MAX instead of INT16_MAX because negative values still
  // count towards the resolution.
  // This value is also using the minimum range this gyroscope supports,
  // because the minimum range is what gives the maximum resolution.
  sensor->resolution = 4.36f / UINT16_MAX;
  sensor->min_delay = 0;
}

void L3G4200D_Unified::setRange(gyroRange_t range) {
  _range = range;
  spiWriteReg(REG_CTRL_4, CTRL4_UPDATE_MSB_AND_LSB_TOGETHER |
                              CTRL4_LSB_AT_LOWER_ADDRESS | range);
}

float L3G4200D_Unified::rangeInRadians() {
  // Divided by 2 because we have both negative and positive values.
  switch (_range) {
  // Intentional fallthrough.
  default:
  case GYRO_RANGE_4_DOT_36_RAD_PER_SEC:
    return 4.36f / 2;

  case GYRO_RANGE_8_DOT_73_RAD_PER_SEC:
    return 8.73f / 2;

  case GYRO_RANGE_34_DOT_91_RAD_PER_SEC:
    return 34.91f / 2;
  }
}

uint8_t L3G4200D_Unified::rawReadReg(uint8_t regAddress) {
  return spiReadReg(regAddress);
}

void L3G4200D_Unified::rawWriteReg(uint8_t regAddress, uint8_t newValue) {
  spiWriteReg(regAddress, newValue);
}

int16_t L3G4200D_Unified::rawX() {
  uint8_t xLow = spiReadReg(REG_OUT_X_L);
  uint8_t xHigh = spiReadReg(REG_OUT_X_H);
  return (int16_t)((xHigh << 8) | xLow);
}

int16_t L3G4200D_Unified::rawY() {
  uint8_t yLow = spiReadReg(REG_OUT_Y_L);
  uint8_t yHigh = spiReadReg(REG_OUT_Y_H);
  return (int16_t)((yHigh << 8) | yLow);
}

int16_t L3G4200D_Unified::rawZ() {
  uint8_t zLow = spiReadReg(REG_OUT_Z_L);
  uint8_t zHigh = spiReadReg(REG_OUT_Z_H);
  return (int16_t)((zHigh << 8) | zLow);
}

rawGyroSample L3G4200D_Unified::rawXYZ() {

  /* L3G4200D SPI read command is:
   * 1 bit:  always set HIGH to indicate we're reading
   * 1 bit:  HIGH indicates auto-increment address across multiple reads;
   *         we're reading 3 register values, so we assert HIGH.
   * 5 bits: The address of the register we want to read from.
   *
   * So the byte we transfer over SPI is the address, but with the two most
   * significant bits set to indicate a register read, and auto-incrementing
   * address.
   * This byte will be the same for each read, even though we're reading
   * different registers, because we're using the auto-increment bit to
   * change which register we're reading.
   */

  uint8_t readCmd = REG_OUT_X_L | 0b11000000;

  beginTransaction();

  // Ignore the first return value, since the gyroscope SPI peripheral hasn't
  // gotten a chance to known what we're asking of it yet.
  _spi->transfer(readCmd);

  // Now start storing values. Remember, the value that's returned by the SPI
  // transfer is the value we asked for in the *last* call to _spi->transfer().
  uint8_t xLow = _spi->transfer(readCmd);
  uint8_t xHigh = _spi->transfer(readCmd);
  uint8_t yLow = _spi->transfer(readCmd);
  uint8_t yHigh = _spi->transfer(readCmd);
  uint8_t zLow = _spi->transfer(readCmd);
  uint8_t zHigh = _spi->transfer(readCmd);

  endTransaction();

  // Now populate the struct with the raw samples.
  rawGyroSample sample;
  sample.x = (int16_t)((xHigh << 8) | xLow);
  sample.y = (int16_t)((yHigh << 8) | yLow);
  sample.z = (int16_t)((zHigh << 8) | zLow);

  // And return it.
  return sample;
}

void L3G4200D_Unified::beginTransaction() {
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_spiCS, LOW);
}

void L3G4200D_Unified::endTransaction() {
  digitalWrite(_spiCS, HIGH);
  _spi->endTransaction();
}

uint8_t L3G4200D_Unified::spiReadReg(uint8_t regAddress) {

  beginTransaction();

  /* L3G4200D SPI read command is:
   * 1 bit:  always set HIGH to indicate we're reading
   * 1 bit:  HIGH indicates auto-increment address across multiple reads, so we
   * assert LOW. 5 bits: The address of the register we want to read from.
   *
   * So the byte we transfer over SPI is the address, but with the most
   * significant bit set to indicate a register read.
   * In this case we don't care about the return value.
   */
  _spi->transfer(regAddress | 0x80);

  // Now that we've sent the address over SPI, perform one more transfer to get
  // the response from the peripheral.
  uint8_t val = _spi->transfer(0);

  endTransaction();

  return val;
}

void L3G4200D_Unified::spiWriteReg(uint8_t regAddress, uint8_t value) {

  beginTransaction();

  /* L3G4200D SPI write command is:
   * 1 bit:  always LOW to indicate we're writing
   * 1 bit:  HIGH indicates auto-increment address across multiple reads, so we
   * assert LOW 5 bits: The address of the register we want to read from. 8
   * bits: The byte that is written to that register.
   *
   * So that's two bytes, one that's just the register address, anod
   * another that's just the value to write to that register.
   */

  _spi->transfer(regAddress);
  _spi->transfer(value);

  endTransaction();
}

float L3G4200D_Unified::sampleToRad(int16_t fullScaleSample) {
  // The gyro chip gives us sample values as a fraction of the full scale.
  // rawSample / INT16_MAX = radValue / gyroRange
  // Solving for radValue:
  // radValue = (rawSample * gyroRange) / INT16_MAX
  return (fullScaleSample * rangeInRadians()) / INT16_MAX;
}
