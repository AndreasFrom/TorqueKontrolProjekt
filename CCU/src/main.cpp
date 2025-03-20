#include <Arduino.h>
#include <Wire.h>
#include <queue>
#include "AGTimerR4.h" //https://github.com/washiyamagiken/AGTimer_R4_Library/tree/main
#include "wifihandler.h" 
#include <DFRobot_BMX160.h>
#include "i2c_master.h"
#include "databuffer.h"

#define SEND_DATA_SERIAL 1

// I2C Config
#define SLAVE_ADDRESS_START 0x08 // Første I2C slaveadresse

// WiFi Config
WiFiHandler wifiHandler("coolguys123", "werty123", 4242);
WiFiClient client;
// data buffer
DataBuffer dataBuffer;
// I2C
I2CMaster i2cMaster;
// IMU
DFRobot_BMX160 bmx160;
bool logging = false; // Flag til logging

volatile bool controlFlag = false; // Flag to indicate when to run the control loop
const double SAMPLE_FREQ = 100.0; //100Hz, 10ms sample time

void handleClientCommunication(WiFiClient &client);
void sendSensorData(WiFiClient &client);
void processClientMessage(String message);

void timerISR() {
    //controlFlag = true; // Set the flag in the ISR
    //Perform measurements and add to queue
    unsigned long timestamp = millis();
    sBmx160SensorData_t Ogyro = {0, 0, 0};
    sBmx160SensorData_t Oaccel = {0, 0, 0};
    bmx160.getGyroACC(&Ogyro, &Oaccel);
    dataBuffer.addData({timestamp, Oaccel.x, Oaccel.y, Ogyro.z});
}

void setup() {
    Serial.begin(115200);
    i2cMaster.begin();
    wifiHandler.connectToWiFi();
    wifiHandler.startTCPServer();

    // Initialize Timer1 to trigger every 10ms
    AGTimer.init(SAMPLE_FREQ, timerISR);
    AGTimer.start();

    if (!bmx160.begin()) {
        Serial.println("Sensor init fejlede!");
        while (1);
    }
}

void loop() {
    client = wifiHandler.acceptClient();
    if (client) {
        Serial.println("Client connected!");
        while (client.connected()) {
            handleClientCommunication(client);
        }
        client.stop();
        logging = false; // Reset logging flag when client disconnects
        Serial.println("Client disconnected.");
    }
}

void handleClientCommunication(WiFiClient &client) {
    if (logging) {
        sendSensorData(client);
    }

    if (client.available()) {
        String message = client.readStringUntil('\n');
        processClientMessage(message);
    }
}

void processClientMessage(String message) {
    Serial.print("Received: ");
    Serial.println(message);

    if (message == "START") {
        client.println("ACK:START");
        logging = true;
        Serial.println("Logging started!");
    } else if (message == "STOP") {
        client.println("ACK:STOP");
        logging = false;
        Serial.println("Logging stopped!");
    } else if (message.startsWith("PID:")) {
        client.println("ACK:PID");
        message.remove(0, 4);
        int comma1 = message.indexOf(',');
        int comma2 = message.indexOf(',', comma1 + 1);
        int comma3 = message.indexOf(',', comma2 + 1);

        if (comma1 == -1 || comma2 == -1 || comma3 == -1) {
            Serial.println("Fejl: Forkert PID-format");
            return;
        }

        float kp = message.substring(0, comma1).toFloat();
        float ki = message.substring(comma1 + 1, comma2).toFloat();
        float kd = message.substring(comma2 + 1, comma3).toFloat();
        float setpoint = message.substring(comma3 + 1).toFloat();

        Serial.print("Parsed PID: ");
        Serial.print("Setpoint: "); Serial.print(setpoint);
        Serial.print(" Kp: "); Serial.print(kp);
        Serial.print(" Ki: "); Serial.print(ki);
        Serial.print(" Kd: "); Serial.println(kd);

        for (int i = 0; i < 4; i++) {
            bool success = i2cMaster.sendData(SLAVE_ADDRESS_START + i, setpoint, kp, ki, kd);
            if (!success) {
                Serial.println("I2C communication failed!");
            }
        }
    } else if (message.startsWith("SETPOINT:")) {
        client.println("ACK:SETPOINT");
        float setpoint = message.substring(9).toFloat();
        Serial.print("Setpoint modtaget: ");
        Serial.println(setpoint);
        for (int i = 0; i < 4; i++) {
            bool success = i2cMaster.sendData(SLAVE_ADDRESS_START + i, setpoint, -1, -1, -1);
            if (!success) {
                Serial.println("I2C communication failed!");
            }
        }
    }
}

void sendSensorData(WiFiClient &client) {

    // Create struct to hold sensor data
    dataBlock data;

    // Check if data is avaliable in queue
    if(dataBuffer.getData(data)){ 
        client.print("SENSOR:");
        client.print("Oaccel: ");
        client.print(data.acc_x); client.print(", ");
        client.print(data.acc_y); client.print(", ");
        client.print("Ogyro: ");
        client.print(data.gyro_z); client.print(" | ");
        client.println();
    }

    if(SEND_DATA_SERIAL){
        Serial.print("Sendt sensor data: ");
        Serial.print("Oaccel: ");
        Serial.print(data.acc_x); client.print(", ");
        Serial.print(data.acc_y); client.print(", ");
        Serial.print("Ogyro: ");
        Serial.print(data.gyro_z); client.print(" | ");
        Serial.println();
    }
}