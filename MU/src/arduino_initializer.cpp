#include "arduino_initializer.h"

ArduinoInitializer::ArduinoInitializer(int sensorPin, int pwmPin, int enablePin, int dirPin,
                                       MotorSensor* sensor, TimerInterrupt* timer)
    : sensorPin_(sensorPin), pwmPin_(pwmPin), enablePin_(enablePin), dirPin_(dirPin),
      sensor_(sensor), timer_(timer) {
    // Validate pin numbers
    if (sensorPin_ < 0 || pwmPin_ < 0 || enablePin_ < 0 || dirPin_ < 0) {
        Serial.println("Error: Invalid pin configuration.");
        while (true); // Halt execution
    }
}

ArduinoInitializer::~ArduinoInitializer() {
    delete sensor_;
    delete timer_;
}

void ArduinoInitializer::begin() {
    initializeMotor();
    initializeSensor();
    initializeTimer();
}

void ArduinoInitializer::initializeMotor() {
    pinMode(pwmPin_, OUTPUT);
    pinMode(enablePin_, OUTPUT);
    pinMode(dirPin_, OUTPUT);

    digitalWrite(enablePin_, HIGH);
    digitalWrite(dirPin_, HIGH);
    analogWrite(pwmPin_, 0); // Initialize with PWM value 0

    Serial.println("Motor initialized.");
}

void ArduinoInitializer::initializeSensor() {
    if (sensor_) {
        sensor_->begin(); // Initialize the sensor
        attachInterrupt(digitalPinToInterrupt(sensorPin_), MotorSensor::MotorSensorISR, RISING);
        Serial.println("Sensor initialized.");
    } else {
        Serial.println("Error: Sensor not provided.");
    }
}

void ArduinoInitializer::initializeTimer() {
    if (timer_) {
        timer_->begin(14); // Start the timer with a 1ms interrupt rate
        timer_->attachInterruptHandler([]() {
            // Timer ISR logic
            static bool toggle1 = false;
            toggle1 = !toggle1;
            if (toggle1) {
                // Perform PID control here if needed
            }
        });
        Serial.println("Timer initialized.");
    } else {
        Serial.println("Error: Timer not provided.");
    }
}