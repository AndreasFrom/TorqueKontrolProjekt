#include <Arduino.h>
#include <Wire.h>

// Pin definitions
#define PWM_PIN 5      
#define ENABLE_PIN 6   
#define DIR_PIN 7      
#define SENSOR_PIN 3   
#define SENSOR_PIN2 4  
#define CURRENT_SENSE A0

//  I2C address
#define I2C_ADDRESS 0x08  

//https://gist.github.com/bradley219/5373998
//https://www.instructables.com/Arduino-Timer-Interrupts/

// Variables for speed calculation
volatile unsigned long lastTime = 0;
volatile unsigned long timeBetweenSensors = 0;  
double currentRPM = 0.0;  
volatile bool sensorTriggered = false;

// Moving average filter for RPM
#define NUM_READINGS 5
double rpmReadings[NUM_READINGS] = {0};
int rpmIndex = 0;

// Motor speed settings
int pwmValue = 0;
const int MIN_PWM = 20;
const int MAX_PWM = 255;

// PID variables
double setpointRPM = 250.0;  
double error = 0.0;
double lastError = 0.0;
double integral = 0.0;
double derivative = 0.0;
double output = 0.0;

// PID tuning parameters
double kp = 1;  
double ki = 2; 
double kd = 0.02;

boolean toggle1 = 0;
unsigned long lastPIDUpdate = 0;

void sensorISR() {
    unsigned long currentMicros = micros();
    if (lastTime != 0) {
        timeBetweenSensors = currentMicros - lastTime;
        sensorTriggered = true;
    }
    lastTime = currentMicros;
}

void receiveEvent(int bytes) {
    if (bytes >= 4) {  // Ensure enough data is received
        setpointRPM = Wire.read() * 10;  // Scale back
        kp = Wire.read() / 10.0;
        ki = Wire.read() / 10.0;
        kd = Wire.read() / 10.0;

        Serial.print("Received: Setpoint = ");
        Serial.print(setpointRPM);
        Serial.print(", Kp = ");
        Serial.print(kp);
        Serial.print(", Ki = ");
        Serial.print(ki);
        Serial.print(", Kd = ");
        Serial.println(kd);
    }
}

void requestEvent() {
    // Send data back to master
    Wire.write((byte)(setpointRPM / 10));  
    Wire.write((byte)(kp * 10));
    Wire.write((byte)(ki * 10));
    Wire.write((byte)(kd * 10));
}

double getFilteredRPM(double newRPM) {
    rpmReadings[rpmIndex] = newRPM;
    rpmIndex = (rpmIndex + 1) % NUM_READINGS;

    double sum = 0;
    for (int i = 0; i < NUM_READINGS; i++) {
        sum += rpmReadings[i];
    }
    return sum / NUM_READINGS;
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

    // Initialize I2C as slave
    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(receiveEvent);  // Handle received data
    Wire.onRequest(requestEvent);  // Send data when requested

    //Timer 1 interrupts
    //set timer1 interrupt at 1Hz
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = 14;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);

}

void loop() {

    if (sensorTriggered) {
        sensorTriggered = false; 

        if (timeBetweenSensors > 0) {
            double rawRPM = (1.0 / (timeBetweenSensors / 10000.0)) * 60.0;
            currentRPM = getFilteredRPM(rawRPM);
        }
    }

    if (toggle1) {
        toggle1 = false;

        // Calculate dt
        unsigned long now = millis();
        double dt = (now - lastPIDUpdate) / 1000.0; // Convert to seconds
        lastPIDUpdate = now;

        // PID calculations
        error = setpointRPM - currentRPM;
        integral += error * (dt);
        //integral = constrain(integral, -50, 50);  // Prevent integral windup
        derivative = (error - lastError) / (dt);
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

    delay(1);
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
    toggle1 = true;
}