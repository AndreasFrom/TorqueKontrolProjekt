#include <Arduino.h>
#include <Wire.h>
#include <queue>
#include <SD.h>
#include "AGTimerR4.h" //https://github.com/washiyamagiken/AGTimer_R4_Library/tree/main
#include "wifihandler.h" 
#include <DFRobot_BMX160.h>
#include "sdLogger.h"
#include "i2c_master.h"


#define SEND_DATA_SERIAL false

// WiFi Config
WiFiHandler wifiHandler("coolguys123", "werty123", 4242);
//WiFiHandler wifiHandler("net", "simsimbims", 4242);
WiFiClient client;

// SD card
const int chipselect = 10;
SDLogger sdLogger;

// I2C
#define SLAVE_ADDRESS_START 0x08 // Første I2C slaveadresse
I2CMaster i2cMaster;

// IMU
DFRobot_BMX160 bmx160;

bool logging = false; // Flag til logging
const double SAMPLE_FREQ = 100.0; //100Hz, 10ms sample time


// Temp data
float kp = 1.0;
float ki = 10.0;
float kd = 0.01;
uint8_t mode = 2;                      
float setpoint = 2.0;                  
float setpoint_radius = 2.0; 


// Prototypes
void handleClientCommunication(WiFiClient &client);
void processClientMessage(String message);

void timerISR() {
    if(logging){
        //Perform measurements and add to queue
        unsigned long timestamp = millis();    

        sBmx160SensorData_t Ogyro = {0, 0, 0};  
        sBmx160SensorData_t Oaccel = {0, 0, 0}; 
        sBmx160SensorData_t Omagn = {0, 0, 0};  
        //bmx160.getGyroACC(&Ogyro, &Oaccel);
        bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
        //sdLogger.addData({timestamp, Oaccel.x, Oaccel.y, Oaccel.z});

        MUData MU0;
        MUData MU1;
        MUData MU2;
        MUData MU3;
        i2cMaster.requestData(SLAVE_ADDRESS_START, MU0);
        i2cMaster.requestData(SLAVE_ADDRESS_START + 1, MU1);
        i2cMaster.requestData(SLAVE_ADDRESS_START + 2, MU2);
        i2cMaster.requestData(SLAVE_ADDRESS_START + 3, MU3);

        sdLogger.addData({
            timestamp, 
            mode, setpoint, setpoint_radius, 
            Oaccel.x, Oaccel.y, Ogyro.z,
            kp, ki, kd,
            MU0, MU1, MU2, MU3
        });
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(chipselect, OUTPUT); // Set the CS pin to output
    
    i2cMaster.begin();
    wifiHandler.connectToWiFi();
    wifiHandler.startTCPServer();
    sdLogger.init(chipselect, "data.csv");

    // Initialize Timer1 to trigger every 10ms
    AGTimer.init(SAMPLE_FREQ, timerISR);
    AGTimer.start();

    if (!bmx160.begin()) {
        Serial.println("Sensor init fejlede!");
        while (1);
    }
    Serial.println("Setup complete!");

    int setpoint0 = (int)(173);
    int setpoint1 = 0;//(int)(205/100);
    int setpoint2 = 0;//(int)(117/100);
    int setpoint3 = 0;//(int)(164/100);
    // Temp until send from commander works
    i2cMaster.sendParam(SLAVE_ADDRESS_START, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, setpoint0);  
    i2cMaster.sendParam(SLAVE_ADDRESS_START+1, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+1, setpoint1);
    i2cMaster.sendParam(SLAVE_ADDRESS_START+2, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+2, setpoint2);
    i2cMaster.sendParam(SLAVE_ADDRESS_START+3, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+3, setpoint3);
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
        sdLogger.close();
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
            bool success = i2cMaster.sendParam(SLAVE_ADDRESS_START + i, setpoint, kp, ki, kd);
            if (!success) {
                Serial.println("I2C communication failed!");
            }
        }

    } else if (message.startsWith("SETPOINT:")) {
        client.println("ACK:SETPOINT");
        setpoint = message.substring(9).toFloat();
        Serial.print("Setpoint modtaget: ");
        Serial.println(setpoint);
        for (int i = 0; i < 4; i++) {
            bool success = i2cMaster.sendParam(SLAVE_ADDRESS_START + i, setpoint, -1, -1, -1);
            if (!success) {
                Serial.println("I2C communication failed!");
            }
        }
    }
}