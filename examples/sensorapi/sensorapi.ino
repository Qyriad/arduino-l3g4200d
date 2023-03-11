#include <L3G4200D_U.h>

/* This driver uses the Adafruit Unified Sensor Library (Adafruit_Sensor),
   which provides a common type and interface for sensor data and helper
   functions.

   To use this library you will also need to download the [Adafruit_Sensor]
   (https://www.arduino.cc/reference/en/libraries/adafruit-unified-sensor/)
   library and include it in your libraries folder.
   Instructions for how to install libraries in the Arduino IDE can be found here
   https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library

   You should assign a unique ID to this sensor for use with the Adafruit
   Sensor API so that you can identify this particular sensor in your code
   and in any data logs. To assign a unique ID, provide a unique value of your
   choosing in the constructor for the L3G4200D_Unified class. Below examples
   use 2113 as the sensor ID.

  Connections
  ===========
  Connect board SCK to sensor SCK.
  Connect board CIPO (MISO) to sensor CIPO (MISO).
  Connect board COPI (MOSI) to sensor COPI (MOSI).
  Connect board 3V3 to sensor Vdd_IO.
  Connect board GND to sensor GND (or a common ground).
  Connect any free board GPIO pin of your choosing to sensor CS (Chip Select).

  This example uses pin 10 as the Chip Select pin.

*/

L3G4200D_Unified gyro = L3G4200D_Unified(2113);

/*
   Displays some basic information about this sensor from the Unified Sensor
   API type sensor_t (see Adafruit_Sensor for more information).
*/
void displayGyroDetails() {
  sensor_t gyroDetails;
  gyro.getSensor(&gyroDetails);
  Serial.println("--------------------------------");
  Serial.print("Sensor:      "); Serial.println(gyroDetails.name);
  Serial.print("Driver Ver:  "); Serial.println(gyroDetails.version);
  Serial.print("Unique ID:   "); Serial.println(gyroDetails.sensor_id);
  Serial.print("Max Value:   "); Serial.print(gyroDetails.max_value); Serial.println(" rad/s");
  Serial.print("Min Value:   "); Serial.print(gyroDetails.min_value); Serial.println(" rad/s");
  Serial.print("Resolution:  "); Serial.print(gyroDetails.resolution); Serial.println(" rad/s");
  Serial.println("--------------------------------");

  delay(500);
}

void configureGyro() {
  /* You can manually set the range by passing it as the second argument to begin: */
  if (!gyro.begin(10, GYRO_RANGE_4_DOT_36_RAD_PER_SEC)) { // 4.36 rad/s.
    /* There was a problem detecting and initalizing the L3G4200D.
       There might be a wiring problem, or maybe Chip Select is not set to
       the correct pin.
    */
    Serial.println("Ooops, no L3G4200D detected. Check your wiring or CS pin.");
    while (1) { }
  }
  // gyro.begin(10, GYRO_RANGE_8_DOT_73_RAD_PER_SEC); // 8.73 rad/s.
  // gyro.begin(10, GYRO_RANGE_34_DOT_91_RAD_PER_SEC); // 34.91 rad/s.

  /* If you do not specify a range, 4.36 rad/s is used. */
  // gyro.begin(10); // 4.36 rad/s.

  /* You may also enable automatic range changing, which causes the gyroscope
     driver to automatically increase the range if it seems like you're
     going past the maximum range of the sensor ("saturating" the sensor).
  */
  gyro.enableAutoRange(true);


  Serial.println("--------------------------------");
  Serial.print("Range: "); Serial.print(gyro.rangeInRadians()); Serial.println(" rad/s");
  Serial.println("--------------------------------");

}

/*
   Arduino setup function (automatically called at startup.
*/
void setup() {

  displayGyroDetails();

  /* Initialize the gyroscope sensor. */
  configureGyro();

  /* The sensor is initialized; we're all set. */
  Serial.println("");
}

/*
   Arduino loop function -- called once setup() is complete.

   This is where your own code should go.
*/
void loop() {
  sensors_event_t gyroEvent;
  gyro.getEvent(&gyroEvent);

  /*
     Print the X, Y, and Z values in columns, and the unit (radians per second)
     at the end.
  */
  Serial.print(gyroEvent.gyro.x);
  Serial.print("\t\t");
  Serial.print(gyroEvent.gyro.y);
  Serial.print("\t\t");
  Serial.print(gyroEvent.gyro.z);
  Serial.println("\t (rad/s)");

  /* Get and print gyroscope data every 300 milliseconds. */
  delay(300);
}
