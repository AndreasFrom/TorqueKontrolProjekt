#include <Arduino.h>
#include "arduino_initializer.h"
#include "motor_pid.h"
#include "timer_interrupt.h"
#include "motor_sensor.h"

// Pin definitions
#define PWM_PIN 5
#define ENABLE_PIN 6
#define DIR_PIN 7
#define SENSOR_PIN 3
#define CURRENT_SENSE A0

// Constants
const int MIN_PWM = 20;
const int MAX_PWM = 255;
const double SAMPLE_TIME = 0.001; // 1ms sample time

// Objects
MotorPID pid(1, 3, 0.0, 300, SAMPLE_TIME); // Example gains and setpoint
TimerInterrupt timer1;
MotorSensor motorSensor(SENSOR_PIN, 5);
ArduinoInitializer arduinoInitializer(SENSOR_PIN, PWM_PIN, ENABLE_PIN, DIR_PIN, &motorSensor, &timer1);

// Variables
double currentRPM = 1;
int pwmValue = 0;
volatile bool controlFlag = false; // Flag to indicate when to run the control logic

void controlLoop() {
    // Process sensor data and calculate RPM
    if (motorSensor.isSensorTriggered()) {
        motorSensor.resetSensorTriggered();
        double rawRPM = ((1.0 / (motorSensor.getTimeBetweenSensors())) * ((6000.0) / 11)) * 1000;
        currentRPM = motorSensor.getFilteredRPM(rawRPM);
    }

    double output = pid.compute(currentRPM);
    pwmValue = constrain(output, MIN_PWM, MAX_PWM);
    analogWrite(PWM_PIN, pwmValue);

    Serial.print(motorSensor.getTimeBetweenSensors());
    Serial.print(",");
    Serial.print(currentRPM);
    Serial.print(",");
    Serial.println(pwmValue);
}

void timerISR() {
    controlFlag = true; // Set the flag in the ISR
}

void setup() {
    Serial.begin(115200);
    delay(10);

    // Initialize all components
    arduinoInitializer.begin();
    
    // Initialize Timer1 with the sample time
    timer1.begin(SAMPLE_TIME);

    // Attach the timer ISR
    timer1.attachInterruptHandler(timerISR);
}

void loop() {
    if (controlFlag) { // Check the flag in the main loop
        controlFlag = false; // Reset the flag
        controlLoop(); // Run the control logic
    }
}

// Timer1 ISR
ISR(TIMER1_COMPA_vect) {
    TimerInterrupt::handleInterrupt();
}