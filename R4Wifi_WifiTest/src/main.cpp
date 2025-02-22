#include <WiFi.h>

/* WiFi Credentials */
const char* ssid = "coolguys123";  // Replace with your Wi-Fi SSID
const char* pwd = "werty123";      // Replace with your Wi-Fi password

/* TCP Config */
const int tcpPort = 4242;  // TCP port to listen on
WiFiServer server(tcpPort);

bool logging = false;  // Flag to control logging

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, pwd);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start TCP server
    server.begin();
    Serial.println("TCP Server is listening...");
}

void loop() {
    // Check for a client connection
    WiFiClient client = server.available();
    if (client) {
        Serial.println("Client connected!");

        // Read data from the client
        while (client.connected()) {
            if (logging) {
                int sensorValue = analogRead(A0);  // Read from analog sensor
                client.print("SENSOR:");
                client.println(sensorValue);
                Serial.print("Sent sensor data: ");
                Serial.println(sensorValue);

                delay(1000);  // Adjust the delay based on your logging needs
            }

            if (client.available()) {
                String message = client.readStringUntil('\n');
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

        }

        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
    }
}