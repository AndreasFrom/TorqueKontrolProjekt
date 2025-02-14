#include <Arduino.h>
#include "motor_sensor.h"

// Pin definitions
#define PWM_PIN 5
#define ENABLE_PIN 6
#define DIR_PIN 7
#define SENSOR_PIN 3
#define CURRENT_SENSE A0
// put function declarations here:

int pwmValue = 150;
MotorSensor motorSensor(SENSOR_PIN, 10);
// Variables
double currentRPM = 1;

void setup() {
  Serial.begin(115200);
  motorSensor.begin();
  //TODO: Move this to motorsensor begin
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), MotorSensor::MotorSensorISR, RISING);
}

void loop() {
  // Process sensor data and calculate RPM
  if (motorSensor.isSensorTriggered()) {
    motorSensor.resetSensorTriggered();
    double rawRPM = ((1.0 / (motorSensor.getTimeBetweenSensors())) * ((6000.0) / 11)) * 1000;
    currentRPM = motorSensor.getFilteredRPM(rawRPM);
  }

  analogWrite(PWM_PIN, pwmValue);

  Serial.print("RPM: ");
  Serial.println(currentRPM);
  delay(100);
}
