#include "wifihandler.h"

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
    
    delay(1000);
}

void sendSensorData(WiFiClient &client) {
    int sensorValue = analogRead(A0);  
    client.print("SENSOR:");
    client.println(sensorValue);
    Serial.print("Sent sensor data: ");
    Serial.println(sensorValue);
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