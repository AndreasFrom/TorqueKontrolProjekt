#include "wifihandler.h"
#include <DFRobot_BMX160.h>
#include <Wire.h>
#include <SPI.h>

DFRobot_BMX160 bmx160;

// Function prototypes
void handleClientCommunication(WiFiClient &client);
void sendSensorData(WiFiClient &client);
void processClientMessage(String message);

// Global variables
WiFiHandler wifiHandler("coolguys123", "werty123", 4242);  // Replace with your credentials
bool logging = false;  // Flag to control logging

void setup() {
    Serial.begin(115200);
    wifiHandler.connectToWiFi();  // Connect to Wi-Fi
    wifiHandler.startTCPServer(); // Start TCP server

      //init the hardware bmx160  
    if (bmx160.begin() != true){
    Serial.println("init false");
    while(1);
    }
}

void loop() {
    // Check for a client connection
    WiFiClient client = wifiHandler.acceptClient();
    if (client) {
        Serial.println("Client connected!");
        handleClientCommunication(client);  //while loop
        client.stop();  // Close the connection
        Serial.println("Client disconnected.");
    }
}

void handleClientCommunication(WiFiClient &client) {
    while (client.connected()) {
        if (logging) {
            sendSensorData(client);  // Send sensor data if logging is enabled
        }

        if (client.available()) {
            String message = client.readStringUntil('\n');
            processClientMessage(message);  // Process the received message
        }
    }
    
    delay(1);
}

void sendSensorData(WiFiClient &client) {
    sBmx160SensorData_t Omagn, Ogyro, Oaccel;
  
    bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);

    client.print("SENSOR:");
    client.print("Omagn: ");
    client.print(Omagn.x); client.print(", ");
    client.print(Omagn.y); client.print(", ");
    client.print(Omagn.z); client.print(" | ");

    client.print("Ogyro: ");
    client.print(Ogyro.x); client.print(", ");
    client.print(Ogyro.y); client.print(", ");
    client.print(Ogyro.z); client.print(" | ");

    client.print("Oaccel: ");
    client.print(Oaccel.x); client.print(", ");
    client.print(Oaccel.y); client.print(", ");
    client.print(Oaccel.z);
    client.println(); // New line for better readability

    Serial.print("Sent sensor data: ");
    Serial.print("Omagn: ");
    Serial.print(Omagn.x); Serial.print(", ");
    Serial.print(Omagn.y); Serial.print(", ");
    Serial.print(Omagn.z); Serial.print(" | ");

    Serial.print("Ogyro: ");
    Serial.print(Ogyro.x); Serial.print(", ");
    Serial.print(Ogyro.y); Serial.print(", ");
    Serial.print(Ogyro.z); Serial.print(" | ");

    Serial.print("Oaccel: ");
    Serial.print(Oaccel.x); Serial.print(", ");
    Serial.print(Oaccel.y); Serial.print(", ");
    Serial.print(Oaccel.z);
    Serial.println(); 

}

void processClientMessage(String message) {
    // Process the received message
    Serial.print("Received: ");
    Serial.println(message);

    if (message == "START") {
        logging = true;
        Serial.println("Logging started!");
    } else if (message == "STOP") {
        logging = false;
        Serial.println("Logging stopped!");
    }
}

