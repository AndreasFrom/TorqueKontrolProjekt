#include <Arduino.h>

// Pin definitions
#define PWM_PIN 5      
#define ENABLE_PIN 6   
#define DIR_PIN 7      
#define SENSOR_PIN 3   
#define SENSOR_PIN2 4  
#define CURRENT_SENSE A0

// Variables for speed calculation
volatile unsigned long lastTime = 0;
volatile unsigned long timeBetweenSensors = 0;  
double currentRPM = 0.0;  
volatile bool sensorTriggered = false;

// Moving average filter for RPM
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

// Motor speed settings
int pwmValue = 0;
const int MIN_PWM = 20;
const int MAX_PWM = 255;

// PID variables
double setpointRPM = 500.0;  
double error = 0.0;
double lastError = 0.0;
double integral = 0.0;
double derivative = 0.0;
double output = 0.0;

// PID tuning parameters
double kp = 1;  
double ki = 0.05; 
double kd = 0.02;  

void sensorISR() {
    unsigned long currentMicros = micros();
    if (lastTime != 0) {
        timeBetweenSensors = currentMicros - lastTime;
        sensorTriggered = true;
    }
    lastTime = currentMicros;
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

    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensorISR, RISING);
}

void loop() {
    if (sensorTriggered) {
        sensorTriggered = false; 

        if (timeBetweenSensors > 0) {
            double rawRPM = (1.0 / (timeBetweenSensors / 10000.0)) * 60.0;
            currentRPM = getFilteredRPM(rawRPM);

            // PID calculations
            error = setpointRPM - currentRPM;
            integral += error * 0.01;
            integral = constrain(integral, -50, 50);  // Prevent integral windup
            derivative = (error - lastError) / 0.01;
            output = (kp * error) + (ki * integral) + (kd * derivative);
            lastError = error;

            // Map output to PWM range
            pwmValue = constrain(output, MIN_PWM, MAX_PWM);
            analogWrite(PWM_PIN, pwmValue);

            //int cs = analogRead(CURRENT_SENSE);
            // Logging
            //Serial.print(cs);
            //Serial.print(",");
            Serial.print(timeBetweenSensors);
            Serial.print(",");
            Serial.print(currentRPM);
            Serial.print(",");
            Serial.println(pwmValue);
            
        }
    }

    delay(10);
}
