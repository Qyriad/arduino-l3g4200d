/*!
 * @file L3G4200D_U.h
 *
 * @mainpage ST @htmlonly L3G4200D @endhtmlonly Gyroscope Unified Sensor
 * Library.
 *
 * @section intro_sec Introduction
 *
 * This is a library for the ST @htmlonly L3G4200D @endhtmlonly gyroscope
 * sensor, found on some boards like the Digilent PmodGYRO or the Waveshare
 * L3G4200D breakout.
 *
 * @section author Author
 *
 * Written by Qyriad <qyriad@qyriad.me>, 2023.
 *
 * @section license License
 *
 * MIT license, all text above must be included in any redistribution.
 *
 * @section usage Usage
 *
 * This library uses Adafruit's
 * [Unified Sensor
 * API](https://learn.adafruit.com/using-the-adafruit-unified-sensor-driver/how-does-it-work).
 *
 * To initialize this sensor, first create an object of L3G4200D_Unified with
 * some number of your choosing that will uniquely identify this sensor in your
 * sketch. Examples in this documentation will use `2113` as the sensor ID.
 * Once you've created the object, call L3G4200D_Unified::begin, passing a
 * the number of the pin you have connected to the gyroscope's SPI
 * Chip Select (CS) pin. You may also optionally pass a specific range (from
 * from ::gyroRange_t) if you want a specific range of values to be available
 * (the default 4.36 radians per second). Using a higher range lowers the
 * resolution of the sensor, and using a lower range increases the resolution
 * of the sensor.
 *
 * You may also call L3G4200D_Unified::getSensor to get some metadata about
 * the sensor, such as its minimum and maximum values, its range, the version
 * of this driver, etc.
 *
 * Once you have called L3G4200D_Unified::begin, you may then call
 * L3G4200D::getEvent as many times as you want to sample gyroscope motion
 * data. This will populate `event.gyro.x`, `event.gyro.y`, and `event.gyro.z`
 * in the object you pass it. Here's an example that prints out the X-axis
 * value to the serial console:
 *
 * @code{.cpp}
 *
 * L3G4200D_Unified gyro = L3G4200D_Unified(2113);
 * // This example uses board pin 10 as the CS (Chip Select).
 * // Make sure whatever pin you choose on your Arduino board is connected
 * // to the SPI CS pin of your gyroscope!
 * gyro.begin(10);
 *
 * sensors_event_t event;
 * gyro.getEvent(&event);
 * Serial.println(event.gyro.x);
 *
 * @endcode
 * See `examples/sensorapi/sensorapi.ino` for a full example.
 *
 * @see L3G4200D_Unified::begin
 * @see L3G4200D_Unified::getEvent
 */

#ifndef L3G4200D_U_H
#define L3G4200D_U_H

#if ARDUINO >= 100
#include "Arduino.h"
#include "Print.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_Sensor.h>
#include <SPI.h>

/*! @defgroup sensor Sensor
 *
 * @brief This contains the types used for typical operation of this L3G4200D
 * sensor.
 */

/*!
 * @defgroup registers Registers
 *
 * @brief This contains advanced functionality for reading and writing to raw
 * gyroscope registers. If you just want to read gyro data, you probably want
 * @ref usage or @ref sensor instead.
 *
 * @see L3G4200D_Unified
 *
 * @{
 */

/*! @name Addresses
 * @anchor reg_addresses
 * @{
 */

/*! @brief The address of the chip ID register. Should always read as `0xd3`.
 *
 * @see L3G42000D_CHIP_ID
 */
#define REG_WHO_AM_I (0x0F)

/*! @brief The address of CTRL_REG1, which is used for bandwidth, data rate, and
 * power selection.
 *
 * @see CTRL1.
 */
#define REG_CTRL_1 (0x20)

/*! @brief The address of CTRL_REG2, which is used for configuring the high pass
 * filter.
 *
 * @see CTRL2.
 */
#define REG_CTRL_2 (0x21)

/*! @brief The address of CTRL_REG3, which is used for configuring the
 * electrical characteristics of the pins on the chip.
 *
 * @see CTRL3.
 */
#define REG_CTRL_3 (0x22)

/*! @brief The address of CTRL_REG4, which is used for setting the gyroscope
 * range.
 *
 * @see CTRL4.
 */
#define REG_CTRL_4 (0x23)

/*! @brief The address of CTRL_REG5, which is used for enabling different kinds
 * of filtering.
 *
 * @see CTRL5.
 */
#define REG_CTRL_5 (0x24)

/*! @brief The address of OUT_X_L, which contains the low byte of the X-axis
 * angular data, as two's complement.
 */
#define REG_OUT_X_L (0x28)

/*! @brief The address of OUT_X_L, which contains the high byte of the X-axis
 * angular data, as two's complement.
 */
#define REG_OUT_X_H (0x29)

/*! @brief The address of OUT_X_H, which contains the low byte of the Y-axis
 * angular data, as two's complement.
 */
#define REG_OUT_Y_L (0x2a)

/*! @brief The address of OUT_Y_L, which contains the high byte of the Y-axis
 * angular data, as two's complement.
 */
#define REG_OUT_Y_H (0x2b)

/*! @brief The address of OUT_X_L, which contains the low byte of the Z-axis
 * angular data, as two's complement.
 */
#define REG_OUT_Z_L (0x2c)

/*! @brief The address of OUT_X_L, which contains the high byte of the Z-axis
 * angular data, as two's complement.
 */
#define REG_OUT_Z_H (0x2d)

/*! @brief The chip ID constant value of [REG_WHO_AM_I](@ref REG_WHO_AM_I):
 * `0xd3`.
 *
 * This should be the only value ever read from @ref REG_WHO_AM_I.
 * If you get a different value, your wires may not be connected correctly.
 */
#define L3G4200D_CHIP_ID (0xd3)

/*!
 * @}
 */

/*!
 * @addtogroup CTRL1
 * @ingroup registers
 *
 * @brief Values for @ref REG_CTRL_1, which is used for bandwidth, data rate,
 * and power selection.
 *
 * @{
 */

/*! @name Output data rate and filtering
 * @anchor rate_filtering
 *
 * These values of @ref REG_CTRL_1 set the output data rate and the "low-pass"
 * filtering bandwidth. The low-pass filtering is for "smoothing out" changes
 * that occur too quickly to be useful.
 *
 * These values can be or'd with other `CTRL1_` values when writing to
 * @ref REG_CTRL_1.
 *
 * @{
 */

// Bits 7:6 set the output data rate, and bits 5:4 set the "low-pass" filtering
// bandwidth.

/*! @brief REG_CTRL_1 value for a 100 Hz data rate and a 12.5 Hz cutoff. */
#define CTRL1_RATE_100HZ_CUTOFF_12HZ5 (0b0000 << 4)

/*! @brief REG_CTRL_1 value for a 100 Hz data rate and a 25 Hz cutoff. */
#define CTRL1_RATE_100HZ_CUTOFF_25HZ (0b0001 << 4)

/*! @brief REG_CTRL_1 value for a 200 Hz data rate and a 12.5 Hz cutoff. */
#define CTRL1_RATE_200HZ_CUTOFF_12HZ5 (0b0100 << 4)

/*! @brief REG_CTRL_1 value for a 200 Hz data rate and a 25 Hz cutoff. */
#define CTRL1_RATE_200HZ_CUTOFF_25HZ (0b0101 << 4)

/*! @brief REG_CTRL_1 value for a 200 Hz data rate and a 50 Hz cutoff. */
#define CTRL1_RATE_200HZ_CUTOFF_50HZ (0b0110 << 4)

/*! @brief REG_CTRL_1 value for a 200 Hz data rate and a 70 Hz cutoff. */
#define CTRL1_RATE_200HZ_CUTOFF_70HZ (0b0111 << 4)

/*! @brief REG_CTRL_1 value for a 400 Hz data rate and a 20 Hz cutoff. */
#define CTRL1_RATE_400HZ_CUTOFF_20HZ (0b1000 << 4)

/*! @brief REG_CTRL_1 value for a 400 Hz data rate and a 2 Hz cutoff. */
#define CTRL1_RATE_400HZ_CUTOFF_25HZ (0b1001 << 4)

/*! @brief REG_CTRL_1 value for a 400 Hz data rate and a 50 Hz cutoff. */
#define CTRL1_RATE_400HZ_CUTOFF_50HZ (0b1010 << 4)

/*! @brief REG_CTRL_1 value for a 400 Hz data rate and a 110 Hz cutoff. */
#define CTRL1_RATE_400HZ_CUTOFF_110HZ (0b1011 << 4)

/*! @brief REG_CTRL_1 value for a 800 Hz data rate and a 30 Hz cutoff. */
#define CTRL1_RATE_800HZ_CUTOFF_30HZ (0b1100 << 4)

/*! @brief REG_CTRL_1 value for a 800 Hz data rate and a 35 Hz cutoff. */
#define CTRL1_RATE_800HZ_CUTOFF_35HZ (0b1101 << 4)

/*! @brief REG_CTRL_1 value for a 800 Hz data rate and a 50 Hz cutoff. */
#define CTRL1_RATE_800HZ_CUTOFF_50HZ (0b1110 << 4)

/*! @brief REG_CTRL_1 value for a 800 Hz data rate and a 110 Hz cutoff. */
#define CTRL1_RATE_800HZ_CUTOFF_110HZ (0b1111 << 4)

/*! @} */ // End member group rate_filtering.

/*! @name Power and axes settings
 * @anchor power_axes
 *
 * These values of @ref REG_CTRL_1 set the power mode of the chip as a whole
 * and for each of the X, Y, and Z axes.
 * Turning off unncesseary axes saves power.
 *
 * These values can be or'd with other `CTRL1_` values when writing to
 * @ref REG_CTRL_1
 *
 * @{
 */

// Bit 3 sets the power mode for the chip as a whole.
// Bit 2 enables or disables the Z-axis.
// Bit 1 enables or disables the Y-axis.
// Bit 0 enables or disables the X-axis.

/*! @brief REG_CTRL_1 value to power down the gyroscope. */
#define CTRL1_POWER_DOWN (0b0000 << 0)

/*! @brief REG_CTRL_1 value to put the gyroscope in sleep mode - on, but with no
 * gyroscope axes enabled.
 */
#define CTRL1_SLEEP (0b1000 << 0)

/*! @brief REG_CTRL_1 value to enable the X-axis only. */
#define CTRL1_X_ONLY (0b1001 << 0)

/*! @brief REG_CTRL_1 value to enable the Y-axis only. */
#define CTRL1_Y_ONLY (0b1010 << 0)

/*! @brief REG_CTRL_1 value to enable the Z-axis only. */
#define CTRL1_Z_ONLY (0b1100 << 0)

/*! @brief REG_CTRL_1 value to enable the X and Y axes only. */
#define CTRL1_XY (0b1011 << 0)

/*! @brief REG_CTRL_1 value to enable the Y and Z axes only. */
#define CTRL1_YZ (0b1110 << 0)

/*! @brief REG_CTRL_1 value to enable the X and Z axes only. */
#define CTRL1_XZ (0b1101 << 0)

/*! @brief REG_CTRL_1 value to enable all axes. This is probably what you want.
 */
#define CTRL1_XYZ (0b1111 << 0)

/*! @} */ // End member group power_axes.
/*! @} */ // End group CTRL1.

/*!
 * @addtogroup CTRL2
 * @ingroup registers
 *
 * @brief Values for @ref REG_CTRL_2, which is used for configuring the
 * high pass filter.
 *
 * @{
 */

/*! @name High pass filter setting
 * @anchor high_pass_divisor
 *
 * @brief Values for @ref REG_CTRL_2 that control the high pass filter.
 *
 * The high pass filter rejects movements that are too slow to be useful.
 * The value used for filtering is expressed as a fraction of the clock rate.
 * To figure out the filter frequency, divide the output data rate by the
 * number at the end of these constants.
 *
 * For example, if you want to detect gestures, you might not be interested
 * in various slow movements (which wouldn't be part of a sharp gesture).
 * If your data rate were 200 Hz, and you don't care about gestures that happen
 * over less than 1 second, then you might want HIGH_PASS_DIV_200,
 * since 200 Hz / 200 = 1 Hz.
 *
 * @{
 */

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by 12.
 */
#define CTRL2_HIGH_PASS_DIV_12 (0b0000 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by 25.
 */
#define CTRL2_HIGH_PASS_DIV_25 (0b0001 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by 50.
 */
#define CTRL2_HIGH_PASS_DIV_50 (0b0010 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by 100.
 */
#define CTRL2_HIGH_PASS_DIV_100 (0b0011 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by 200.
 */
#define CTRL2_HIGH_PASS_DIV_200 (0b0100 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by 500.
 */
#define CTRL2_HIGH_PASS_DIV_500 (0b0101 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by
 * 1000. */
#define CTRL2_HIGH_PASS_DIV_1000 (0b0110 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by
 * 2000. */
#define CTRL2_HIGH_PASS_DIV_2000 (0b0111 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by
 * 5000. */
#define CTRL2_HIGH_PASS_DIV_5000 (0b1000 << 0)

/*! @brief REG_CTRL_2 value for filtering based on the data rate divided by
 * 10000. */
#define CTRL2_HIGH_PASS_DIV_10000 (0b1001 << 0)

/*! @} */ // End member group high_pass_divisor
/*! @} */ // End group CTRL2

/*!
 * @addtogroup CTRL3
 * @ingroup registers
 *
 * @brief Values for @ref REG_CTRL_3, which is used for configuring the
 * electrical characteristics of the pins on the chip.
 *
 * @{
 */

/*! @brief REG_CTRL_3 value to indicate that the gyro chip should drive output
 * pins HIGH and LOW, instead of using a pull-up resistor for logic HIGH.
 */
#define CTRL3_DRIVE_HIGH_AND_LOW (0b0 << 4)

/*! @brief REG_CTRL_3 value to indicate that the gyro chip should not drive
 * output pins HIGH, and instead use a pull-up resistor for logic HIGH.
 */
#define CTRL3_USE_PULL_UP_FOR_HIGH (0b1 << 4)

/*! @} */ // End group CTRL3.

/*!
 * @addtogroup CTRL4
 * @ingroup registers
 *
 * @brief Values for @ref REG_CTRL_4, which is used for setting the gyroscope
 * range.
 *
 * @{
 */

/*! @name Output register configuration
 * @anchor out_reg_config
 *
 * These values of @ref REG_CTRL_4 control how the X, Y, and Z axis output
 * registers contain and update their values.
 *
 * These values can be or'd with other `CTRL_4` values when writing to
 * @ref REG_CTRL_4.
 *
 * @{
 */

/*! @brief REG_CTRL_4 value to indicate that the high byte and low byte of each
 * output register should not update when we've read one but not the other.
 */
#define CTRL4_UPDATE_MSB_AND_LSB_TOGETHER (0b1 << 7)

/*! @brief REG_CTRL_4 value to indicate that the low byte of each output
 * register is at the lower address (as is displayed in the datasheet).
 */
#define CTRL4_LSB_AT_LOWER_ADDRESS (0b0 << 6)

/*! @brief REG_CTRL_4 value to indicate that the high byte of each output
 * register is at the higher address.
 */
#define CTRL4_MSB_AT_LOWER_ADDRESS (0b1 << 6)

/*! @} */ // End member group out_reg_config.

/*! @name Gyro range settings
 * @anchor gyro_range
 *
 * These values of @ref REG_CTRL_4 control the current range of the gyroscope
 *
 * These values can be or'd with other `CTRL4_` values when writing to
 * @ref REG_CTRL_4
 *
 * @see ::gyroRange_t
 *
 * @{
 */

/*! @brief REG_CTRL_4 value for a gyroscope range of 250 deg/s. Corresponds to
 * ::GYRO_RANGE_4_DOT_36_RAD_PER_SEC (4.36 rad/s).
 */
#define CTRL4_FULL_SCALE_250DPS (0b00 << 4)

/*! @brief REG_CTRL_4 value for a gyroscope range of 500 deg/s. Corresponds to
 * ::GYRO_RANGE_8_DOT_73_RAD_PER_SEC (8.73 rad/s).
 */
#define CTRL4_FULL_SCALE_500DPS (0b01 << 4)

/*! @brief REG_CTRL_4 value for a gyroscope range of 2000 deg/s. Corresponds to
 * ::GYRO_RANGE_34_DOT_91_RAD_PER_SEC (34.91 rad/s).
 */
#define CTRL4_FULL_SCALE_2000DPS (0b10 << 4)

/*! @} */ // End member group gyro_range.

/*! @} */ // End group CTRL4.

/*!
 * @addtogroup CTRL5
 * @ingroup registers
 *
 * @brief Values for @ref REG_CTRL_5, which is used for enabling different
 * kinds of filtering.
 *
 * @{
 */

/*! @brief REG_CTRL_5 value to disable both the low pass and high pass filter.
 */
#define CTRL5_NO_FILTERING ((0b00 << 0) | (0b0 << 4))

/*! @brief REG_CTRL_5 value to enable the high pass filter only. */
#define CTRL5_HIGH_PASS_FILTERING ((0b01 << 0) | (0b0 << 4))

/*! @brief REG_CTRL_5 value to enable the low pass filter only. */
#define CTRL5_LOW_PASS_FILTERING ((0b10 << 0) | (0b0 << 4))

/*! @brief REG_CTRL_5 value to enable both the band pass filter (both high pass
 * and low pass).
 */
#define CTRL5_BAND_PASS_FILTERING ((0b10 << 0) | (0b1 << 4))

// End group CTRL5.
/*!
 * @}
 */

// End group registers.
/*!
 * @}
 */

/*! @private */
typedef struct rawGyroSample {
  int16_t x; /*!< @private */
  int16_t y; /*!< @private */
  int16_t z; /*!< @private */
} gyroSample_t;

/*!
 * @ingroup sensor
 * @{
 */

/*!
 * @brief Optional sensititity settings. If not specified in
 * L3G4200D_Unified::begin, defaults to ::GYRO_RANGE_4_DOT_36_RAD_PER_SEC.
 *
 * Using a higher range lowers the resolution of the sensor, and using a lower
 * range increases the resolution of the sensor.
 */
typedef enum {
  /*! A range of 4.36 rad/s, or 250 deg/s. */
  GYRO_RANGE_4_DOT_36_RAD_PER_SEC = CTRL4_FULL_SCALE_250DPS,

  /*! A range of 8.73 rad/s, or 500 deg/s. */
  GYRO_RANGE_8_DOT_73_RAD_PER_SEC = CTRL4_FULL_SCALE_500DPS,

  /*! A range of 34.91 rad/s, or 2000 deg/s. */
  GYRO_RANGE_34_DOT_91_RAD_PER_SEC = CTRL4_FULL_SCALE_2000DPS,
} gyroRange_t;

/*!
 * @brief Class for interfacing with an L3G4200D gyroscope, using the Adafruit
 * Unified Sensor API. Most common methods: L4G4200D_Unified::begin and
 * L3G4200D_Unified::getEvent.
 *
 * @ingroup sensor
 */
class L3G4200D_Unified : public Adafruit_Sensor {

public:
  /*! @brief Create a new object representing an @htmlonly L3G4200D @endhtmlonly
   * gyroscope.
   *
   * @param sensorId A number to uniquely identify this sensor. This number
   * can be arbitrarily chosen by you, but should not be shared with any
   * other sensors in your sketch.
   */
  L3G4200D_Unified(int32_t sensorId);

  // Default to the global default SPI connector, which is often brought out
  // and labeled as labeled SPI connectors on Arduino boards.
  /*! @brief Initializes this L3G4200D gyroscope using SPI.
   *
   * @param spiChipSelect
   * @parblock
   * The pin number on your board that you have connected
   * to the SPI CS (Chip Select) pin on the L3G4200D. For example, on the
   * [Digilent PmodGYRO](https://digilent.com/reference/pmod/pmodgyro/start),
   * CS is pin 1 on the J1 (the first jumper). If you connected that pin to
   * pin 10 on your Arduino board, then you would call L4G4200D_Unified::begin
   * like this:
   *
   * @code{.cpp}
   * L3G4200D_Unified gyroscope = L3G4200D(2113);
   * gyroscope.begin(10);
   * @endcode
   *
   * @endparblock
   *
   * @param range One of the ::gyroRange_t values to use for this gyroscope.
   * Defaults to 4.36 rad/s.
   * @param spi
   * @parblock
   * The SPI interface to use when communicating to this gyroscope.
   * Defaults to [SPI], the default SPI interface on Arduino boards. This
   * is often connected to pins labeled `SCK`, `MOSI`, and `MISO` on the
   * physical board. For example, on Arduino Uno the MISO of the default `SPI`
   * interface is pin 12.
   * [SPI]: https://docs.arduino.cc/learn/communication/spi
   * @endparblock
   * @param spiFrequency The clock frequency for the SPI peripheral. Defaults
   * to 5 MHz if not specified. Must be lower than 10 MHz, per the L3G4200D
   * datasheet.
   *
   * @returns True if this sensor was successfully activated, false if it was
   * not. If false, you can use @ref enableDebugLogging to potentially get
   * information on why logged in your serial console.
   */
  bool begin(int spiChipSelect,
             gyroRange_t range = GYRO_RANGE_4_DOT_36_RAD_PER_SEC,
             SPIClass &spi = SPI, uint32_t spiFrequency = 5 * 1000 * 1000);

  /*! @brief Enables automatic range increasing if the sensor seems to be
   * saturating its current range.
   *
   * The range will not automatically decrease after being increased.
   *
   * @param enabled Set to true to enable auto range, false to disable it.
   */
  void enableAutoRange(bool enabled);

  /*! @brief Enables or disables debug logging to the Serial console.
   *
   * @param enabled Set to true to enable debug logging, false to disable it.
   */
  void enableDebugLogging(bool enabled);

  /*! @brief The Unified Sensor API method to get data from this sensor.
   *
   * @param event [out] A pointer to a sensors_event_t object for this method to
   * populate with the X, Y, and Z gyro data.
   *
   * After this function is called, `event->gyro.x`, `event->gyro.y`, and
   * `event->gyro.z` are set to the values from the sensor.
   * For example:
   *
   *     L3G4200D_U gyro;
   *     gyro.begin(10);
   *     sensors_event_t event;
   *     gyro.getEvent(&event);
   *     Serial.print("X, Y, Z: ");
   *     Serial.print(event.gyro.x);
   *     Serial.print(", ");
   *     Serial.print(event.gyro.y);
   *     Serial.print(", ");
   *     Serial.println(event.gyro.z);
   *
   * @returns True if this sensor was successfully read from, false if it was
   * not.
   */
  bool getEvent(sensors_event_t *event);

  /*! @brief The Unified Sensor API method to get information about this sensor.
   * @param sensor [out] A pointer to a sensor_t object for this method to
   * populate with information about this sensor.
   */
  void getSensor(sensor_t *sensor);

  /*! @brief Sets the range for this gyroscope.
   * @param range One of the values of gyroRange_t to set as the new range
   * for this gyroscope.
   */
  void setRange(gyroRange_t range);

  /*! @brief Returns the full scale of the current range in the SI unit rad/s.
   * @returns The current range, in radians.
   */
  float rangeInRadians();

  /*! @brief Advanced functionality: reads a raw value from a raw address.
   * See @ref registers for more information.
   *
   * @param regAddress The address of the gyroscope register to read from.
   * One of the values of [register addresses](@ref reg_addresses).
   *
   * @returns The raw value of the register that was read.
   */
  uint8_t rawReadReg(uint8_t regAddress);

  /*! @brief Advanced functionality: writes a raw value to a raw address.
    See @ref registers for more information.
   *
   * @param regAddress The address of the gyroscope register to read from.
   * One of the values of [register addresses](@ref reg_addresses).
   *
   * @param newValue The raw value to write to the register specified in @p
   regAddress.
   */
  void rawWriteReg(uint8_t regAddress, uint8_t newValue);

private:
  SPIClass *_spi;
  int _spiCS;
  int32_t _sensorId;
  bool _autoRangeEnabled;
  gyroRange_t _range;
  SPISettings _spiSettings;
  bool _debugLoggingEnabled;

  /*! @brief Reads the raw sample for the X-axis. */
  int16_t rawX();

  /*! @brief Reads the raw sample for the Y-axis. */
  int16_t rawY();

  /*! @brief Reads the raw sample for the Z-axis. */
  int16_t rawZ();

  /*! @brief Reads samples for the X, Y, and Z axes all at once as one
   * transaction. */
  rawGyroSample rawXYZ();

  /*! @brief Starts an Arduino SPI transaction, and asserts Chip Select. */
  void beginTransaction();

  /*! @brief De-asserts Chip Select, and ends the Arduino SPI transaction. */
  void endTransaction();

  /*! @brief Starts a transaction, reads a register, and ends the transaction.
   */
  uint8_t spiReadReg(uint8_t regAddress);

  /*! @brief Starts a transaction, writes to a register, and ends the
   * transaction. */
  void spiWriteReg(uint8_t regAddress, uint8_t value);

  /*! @brief Converts a raw sample to the SI unit radians per second (rad/s). */
  float sampleToRad(int16_t fullScaleSample);

  /*! @brief Logs if enabled with @ref enableDebugLogging, prepending the sensor
   * ID first. */
  void debugLog(const char str[]);

  /*! @brief Logs if enabled with @ref enableDebugLogging, prepending the sensor
   * ID first. */
  void debugLog(int val);

  /*! @brief Logs if enabled with @ref enableDebugLogging, without prepending
   * the sensor ID first. */
  void debugAppend(const char str[]);

  /*! @brief Logs if enabled with @ref enableDebugLogging, without prepending
   * the sensor ID first. */
  void debugAppend(int val);
};

/*! @} */ // End group sensor.

#endif
