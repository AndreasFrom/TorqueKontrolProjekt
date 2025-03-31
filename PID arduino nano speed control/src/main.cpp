#include <Arduino.h>
#include "arduino_initializer.h"
#include "motor_pid.h"
#include "timer_interrupt.h"
#include "motor_sensor.h"
#include "i2c_slave.h"

// Pin definitions
#define PWM_PIN 5
#define ENABLE_PIN 6
#define DIR_PIN 7
#define SENSOR_PIN 3
#define CURRENT_SENSE A0

// I2C address
#define I2C_ADDRESS 0x08

// Motor speed settings
const int MIN_PWM = 0;
const int MAX_PWM = 255;
const double SAMPLE_TIME = 0.001; // 1ms sample time

// Physical dimensions
#define WHEEL_DIA 0.068 //68mm
#define PI 3.1415926535897932384626433832795

// Variables
double currentRPM = 1;
double currentTorque = 1; 
double currentVelocity = 0.01;
double motorCurrent = 1;
int pwmValue = 0;
volatile bool controlFlag = false; // Flag to indicate when to run the control logic
bool newPIDGainsAvailable = false; // Flag to indicate new PID gains are available
int step = 0;

// Objects
MotorPID pid(0.3, 5, 0.0, 0, SAMPLE_TIME); // Example gains and setpoint
TimerInterrupt timer1;
MotorSensor motorSensor(SENSOR_PIN, 5, CURRENT_SENSE, 5);
ArduinoInitializer arduinoInitializer(SENSOR_PIN, PWM_PIN, ENABLE_PIN, DIR_PIN, &motorSensor, &timer1);
I2CSlave i2cSlave(currentVelocity, currentTorque, currentRPM, motorCurrent);

void controlLoop() {
    // Process sensor data and calculate RPM, Velocity
    if (motorSensor.isSensorTriggered()) {
        motorSensor.resetSensorTriggered();
        double rawRPM = ((1e6*60.0) / 110) / (motorSensor.getTimeBetweenSensors());
        if (rawRPM > 2000) {rawRPM = 0;}

        currentRPM = motorSensor.getFilteredRPM(rawRPM);
        currentVelocity = (currentRPM * PI * WHEEL_DIA) / 60;
    }

    // Torque control
    motorCurrent = motorSensor.getFilteredCurrent(motorSensor.getMotorCurrent());
    currentTorque = 0.0981 * motorCurrent;

    // Compute PID based on mode
    double output = 0;
    switch (i2cSlave.getCtrlMode()) {
        case 0 : // Speed control
            output = pid.compute(currentRPM); //Changed to use RPM internally
            break;

        case 1 : // Torque control
            output = pid.compute(currentTorque);
            break;

        case 2 : // RPM control (Secret mode)
            output = pid.compute(currentRPM);
            break;

        default:
            output = 0;
            break;
    }
    
    pwmValue = constrain(output, MIN_PWM, MAX_PWM);
    analogWrite(PWM_PIN, pwmValue);


    //unsigned long timestamp = millis(); 

    //Serial.print(timestamp); 
    //Serial.print(",");
    //Serial.print(motorSensor.getTimeBetweenSensors());
    //Serial.print("RPM ");
    //Serial.print(currentRPM);
    //Serial.print(",");
    //Serial.println(currentVelocity);
    //Serial.print(",");
    //Serial.println(pwmValue);
    //Serial.print(",");
    //Serial.println(motorCurrent);
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

    // Initialize I2C Slave
    i2cSlave.begin();
}

void loop() {

    if (controlFlag) { // Check the flag in the main loop
        controlFlag = false; // Reset the flag
        controlLoop(); // Run the control logic
    }

    // Update setpoint RPM
    pid.setSetpoint(i2cSlave.getSetpoint());

    // Update PID gains only if new values are available
    if (i2cSlave.newPIDGainsAvailable) {
        pid.setGains(i2cSlave.getCtrlMode(),i2cSlave.getKp(), i2cSlave.getKi(), i2cSlave.getKd());
        i2cSlave.newPIDGainsAvailable = false; // Reset the flag
    }
}

// Timer1 ISR
ISR(TIMER1_COMPA_vect) {
    TimerInterrupt::handleInterrupt();
}