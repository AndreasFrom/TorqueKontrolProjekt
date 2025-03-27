/*!
 * @file readAllData.ino
 * @brief Through the example, you can get the sensor data by using getSensorData:
 * @n     get all data of magnetometer, gyroscope, accelerometer.
 * @n     With the rotation of the sensor, data changes are visible.
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [luoyufeng] (yufeng.luo@dfrobot.com)
 * @maintainer [Fary](feng.yang@dfrobot.com)
 * @version  V1.0
 * @date  2021-10-20
 * @url https://github.com/DFRobot/DFRobot_BMX160
 */
#include <DFRobot_BMX160.h>
#include <Wire.h>
#include <SPI.h>
#include "SimpleKalmanFilter.h"

DFRobot_BMX160 bmx160;

// Create Kalman filters for each axis with appropriate parameters
// Gyroscope: 0.07 °/s noise
SimpleKalmanFilter gyroFilterX(0.07, 1.0, 0.01);
SimpleKalmanFilter gyroFilterY(0.07, 1.0, 0.01);
SimpleKalmanFilter gyroFilterZ(0.07, 1.0, 0.01);

// Accelerometer: 1.8mg noise (0.01766 m/s²)
SimpleKalmanFilter accelFilterX(0.01766, 1.0, 0.01);
SimpleKalmanFilter accelFilterY(0.01766, 1.0, 0.01);
SimpleKalmanFilter accelFilterZ(0.01766, 1.0, 0.01);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  if (bmx160.begin() != true) {
    Serial.println("BMX160 initialization failed");
    while(1);
  }
  delay(100);
}

void loop() {
  sBmx160SensorData_t magn, gyro, accel;

  // Get sensor data
  bmx160.getAllData(&magn, &gyro, &accel);

  // Apply Kalman filtering
  float filteredGyroX = gyroFilterX.updateEstimate(gyro.x);
  float filteredGyroY = gyroFilterY.updateEstimate(gyro.y);
  float filteredGyroZ = gyroFilterZ.updateEstimate(gyro.z);

  float filteredAccelX = accelFilterX.updateEstimate(accel.x);
  float filteredAccelY = accelFilterY.updateEstimate(accel.y);
  float filteredAccelZ = accelFilterZ.updateEstimate(accel.z);

/*   // Print magnetometer data (raw)
  Serial.print("MAG ");
  Serial.print("X: "); Serial.print(magn.x, 2); Serial.print(" uT\t");
  Serial.print("Y: "); Serial.print(magn.y, 2); Serial.print(" uT\t");
  Serial.print("Z: "); Serial.print(magn.z, 2); Serial.println(" uT");

  // Print gyroscope data (raw and filtered)
  Serial.print("GYR RAW ");
  Serial.print("X: "); Serial.print(gyro.x, 2); Serial.print(" °/s\t");
  Serial.print("Y: "); Serial.print(gyro.y, 2); Serial.print(" °/s\t");
  Serial.print("Z: "); Serial.print(gyro.z, 2); Serial.println(" °/s");

  Serial.print("GYR FIL ");
  Serial.print("X: "); Serial.print(filteredGyroX, 2); Serial.print(" °/s\t");
  Serial.print("Y: "); Serial.print(filteredGyroY, 2); Serial.print(" °/s\t");
  Serial.print("Z: "); Serial.print(filteredGyroZ, 2); Serial.println(" °/s");
  
  // Print accelerometer data (raw and filtered)
  Serial.print("ACC RAW ");
  Serial.print("X: "); Serial.print(accel.x, 2); Serial.print(" m/s²\t");
  Serial.print("Y: "); Serial.print(accel.y, 2); Serial.print(" m/s²\t");
  Serial.print("Z: "); Serial.print(accel.z, 2); Serial.println(" m/s²");

  Serial.print("ACC FIL ");
  Serial.print("X: "); Serial.print(filteredAccelX, 2); Serial.print(" m/s²\t");
  Serial.print("Y: "); Serial.print(filteredAccelY, 2); Serial.print(" m/s²\t");
  Serial.print("Z: "); Serial.print(filteredAccelZ, 2); Serial.println(" m/s²"); */

  String data =
    String(accel.x) + "," +
    String(accel.y) + "," +
    String(gyro.z) + "," +
    String(filteredAccelX) + "," +
    String(filteredAccelY) + "," +
    String(filteredGyroZ);

  Serial.print(data);


  Serial.println("");
  delay(100);
}