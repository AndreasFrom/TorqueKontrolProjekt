#include "arduino_initializer.h"

// Constructor with dependency injection
ArduinoInitializer::ArduinoInitializer(int sensorPin, int pwmPin, int enablePin, int dirPin,
                                       Sensor* sensor, TimerInterrupt* timer)
    : sensorPin_(sensorPin), pwmPin_(pwmPin), enablePin_(enablePin), dirPin_(dirPin),
      sensor_(sensor), timer_(timer) {
    // Validate pin numbers
    if (sensorPin_ < 0 || pwmPin_ < 0 || enablePin_ < 0 || dirPin_ < 0) {
        Serial.println("Error: Invalid pin configuration.");
        while (true); // Halt execution
    }
}

// Destructor to clean up dynamically allocated memory
ArduinoInitializer::~ArduinoInitializer() {
    delete sensor_;
    delete timer_;
}

// Initialize all components
void ArduinoInitializer::begin() {
    initializeMotor();
    initializeSensor();
    initializeTimer();
}

// Initialize the motor control pins
void ArduinoInitializer::initializeMotor() {
    pinMode(pwmPin_, OUTPUT);
    pinMode(enablePin_, OUTPUT);
    pinMode(dirPin_, OUTPUT);

    digitalWrite(enablePin_, HIGH);
    digitalWrite(dirPin_, HIGH);
    analogWrite(pwmPin_, 0); // Initialize with PWM value 0

    Serial.println("Motor initialized.");
}

// Initialize the sensor and attach the ISR
void ArduinoInitializer::initializeSensor() {
    if (sensor_) {
        sensor_->begin(); // Initialize the sensor
        attachInterrupt(digitalPinToInterrupt(sensorPin_), Sensor::sensorISR, RISING);
        Serial.println("Sensor initialized.");
    } else {
        Serial.println("Error: Sensor not provided.");
    }
}

// Initialize the timer interrupt
void ArduinoInitializer::initializeTimer() {
    if (timer_) {
        timer_->begin(14); // Start the timer with a 1Hz interrupt rate
        Serial.println("Timer initialized.");
    } else {
        Serial.println("Error: Timer not provided.");
    }
}