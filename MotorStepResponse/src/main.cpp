#include <Arduino.h>
#include "motor_sensor.h"

// Pin definitions
#define PWM_PIN 5
#define ENABLE_PIN 6
#define DIR_PIN 7
#define SENSOR_PIN 3
#define CURRENT_SENSE A0
// put function declarations here:

int pwmValue = 0;
MotorSensor motorSensor(SENSOR_PIN, 5);
// Variables
double currentRPM = 0;
int steps = 0;
double rawRPM = 0;

void setup() {
  Serial.begin(115200);
  motorSensor.begin();
  //TODO: Move this to motorsensor begin
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), MotorSensor::MotorSensorISR, RISING);
}

void loop() {
  
  if (steps == 200) {
    pwmValue = 255;
  }
  
  // Process sensor data and calculate RPM
  if (motorSensor.isSensorTriggered()) {
    motorSensor.resetSensorTriggered();
    rawRPM = ((1.0 / (motorSensor.getTimeBetweenSensors())) * ((6000.0) / 11)) * 1000;
    //currentRPM = motorSensor.getFilteredRPM(rawRPM);
  }
  
  unsigned long timestamp = millis(); 

  Serial.print(timestamp); 
  Serial.print(",");
  Serial.print(motorSensor.getTimeBetweenSensors());
  Serial.print(",");
  Serial.print(rawRPM);
  Serial.print(",");
  Serial.println(pwmValue);

  analogWrite(PWM_PIN, pwmValue);

  steps++;
}
