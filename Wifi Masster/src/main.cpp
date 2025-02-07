#include <Arduino.h>
#include <Wire.h>


#define SLAVE_ADDRESS 0x08  // Must match the slave's address

void setup() {
    Serial.begin(115200);
    Wire.begin();  // Initialize I2C as master
    Serial.println("I2C Master Ready!");
}


void loop() {
    if (Serial.available()) {
        Serial.println("Enter values: Setpoint (RPM), Kp, Ki, Kd (comma-separated)");

        // Read user input
        String input = Serial.readStringUntil('\n');
        input.trim();

        Serial.print("Raw input: ");
        Serial.println(input);  

        // Split input string into 4 values
        String values[4];
        int index = 0;

        while (input.length() > 0 && index < 4) {
            int commaIndex = input.indexOf(',');
            if (commaIndex == -1) {
                values[index] = input;
                break;
            }
            values[index] = input.substring(0, commaIndex);
            input = input.substring(commaIndex + 1);
            index++;
        }

        if (index < 3) {
            Serial.println("Parsing failed! Ensure correct format: 1500,1.0,2.0,0.02");
            return;
        }

        // Convert values to float
        float setpoint = values[0].toFloat();
        float kp = values[1].toFloat();
        float ki = values[2].toFloat();
        float kd = values[3].toFloat();

        // Debugging output
        Serial.print("Parsed: Setpoint = ");
        Serial.print(setpoint);
        Serial.print(", Kp = ");
        Serial.print(kp);
        Serial.print(", Ki = ");
        Serial.print(ki);
        Serial.print(", Kd = ");
        Serial.println(kd);

        // Scale values for transmission
        byte setpoint_scaled = (byte)(setpoint / 10);
        byte kp_scaled = (byte)(kp * 10);
        byte ki_scaled = (byte)(ki * 10);
        byte kd_scaled = (byte)(kd * 10);

        // Send data to slave
        Wire.beginTransmission(SLAVE_ADDRESS);
        Wire.write(setpoint_scaled);
        Wire.write(kp_scaled);
        Wire.write(ki_scaled);
        Wire.write(kd_scaled);
        Wire.endTransmission();

        Serial.println("Data sent successfully!");

        // Request the data back from the slave
        delay(50); // Small delay to allow processing
        Wire.requestFrom(SLAVE_ADDRESS, 4);  

        if (Wire.available() >= 4) {
            int setpoint_recv = Wire.read() * 10;
            float kp_recv = Wire.read() / 10.0;
            float ki_recv = Wire.read() / 10.0;
            float kd_recv = Wire.read() / 10.0;

            Serial.print("Received back: Setpoint = ");
            Serial.print(setpoint_recv);
            Serial.print(", Kp = ");
            Serial.print(kp_recv);
            Serial.print(", Ki = ");
            Serial.print(ki_recv);
            Serial.print(", Kd = ");
            Serial.println(kd_recv);
        } else {
            Serial.println("Error: Did not receive expected data from slave!");
        }
    }
}
