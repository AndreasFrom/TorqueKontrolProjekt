#include <Arduino.h>
#include "motor_pid.h"
#include "timer_interrupt.h"
#include "motor_sensor.h"

// Pin definitions
#define PWM_PIN 5      
#define ENABLE_PIN 6   
#define DIR_PIN 7      
#define SENSOR_PIN 3   
#define CURRENT_SENSE A0

MotorPID pid(1, 2, 0.02);
TimerInterrupt timer1;  // Create a TimerInterrupt object

volatile unsigned long lastTime = 0;
volatile unsigned long timeBetweenSensors = 0;
double currentRPM = 0.0;
volatile bool sensorTriggered = false;

#define NUM_READINGS 5
double rpmReadings[NUM_READINGS] = {0};
int rpmIndex = 0;

double getFilteredRPM(double newRPM) {
    rpmReadings[rpmIndex] = newRPM;
    rpmIndex = (rpmIndex + 1) % NUM_READINGS;
    
    double sum = 0;
    for (int i = 0; i < NUM_READINGS; i++) {
        sum += rpmReadings[i];
    }
    return sum / NUM_READINGS;
}

int pwmValue = 0;
const int MIN_PWM = 20;
const int MAX_PWM = 255;

volatile bool toggle1 = false;

// Sensor ISR function
void sensorISR() {
    unsigned long currentMicros = micros();
    if (lastTime != 0) {
        timeBetweenSensors = currentMicros - lastTime;
        sensorTriggered = true;
    }
    lastTime = currentMicros;
}

// Timer interrupt function
void timerISR() {
    toggle1 = true;
}

void setup() {
    Serial.begin(115200);

    pinMode(PWM_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(SENSOR_PIN, INPUT);
    pinMode(CURRENT_SENSE, INPUT);

    digitalWrite(ENABLE_PIN, HIGH);
    digitalWrite(DIR_PIN, HIGH);
    analogWrite(PWM_PIN, MIN_PWM);

    // Attach interrupt for sensor pin
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensorISR, RISING);

    // Initialize Timer1 with a 1ms interrupt (15625 compare match value for 1ms interval)
    timer1.begin(14);  // Set the timer for 1ms interrupt with a prescaler of 1024
    timer1.attachInterruptHandler(timerISR);  // Attach the ISR for the timer interrupt

    // Initialize other components (if needed)
    // arduinoInit.begin(); // Uncomment if you are using ArduinoInitializer
}

void loop() {
    // Process the sensor data and calculate RPM
    if (sensorTriggered) {
        sensorTriggered = false;
        if (timeBetweenSensors > 0) {
            double rawRPM = ((1.0 / (timeBetweenSensors)) * ((6000.0) / 11)) * 1000;
            currentRPM = getFilteredRPM(rawRPM);
        }
    }

    // If timer interrupt is triggered, calculate PWM value
    if (toggle1) {
        toggle1 = false;
        double output = pid.compute(currentRPM);
        pwmValue = constrain(output, MIN_PWM, MAX_PWM);
        analogWrite(PWM_PIN, pwmValue);

        // Serial output for debugging
        Serial.print(timeBetweenSensors);
        Serial.print(",");
        Serial.print(currentRPM);
        Serial.print(",");
        Serial.println(pwmValue);
    }

    delay(1);
}

// Timer1 ISR function, which calls the interrupt handler in the TimerInterrupt class
ISR(TIMER1_COMPA_vect) {
    TimerInterrupt::handleInterrupt();  // Handle the interrupt and call the user-defined ISR
}
