#include <WiFi.h>

/* WiFi Credentials */
const char* ssid = "coolguys123";  // Replace with your Wi-Fi SSID
const char* pwd = "werty123";      // Replace with your Wi-Fi password

/* TCP Config */
const int tcpPort = 4242;  // TCP port to listen on
WiFiServer server(tcpPort);

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
            if (client.available()) {
                String message = client.readStringUntil('\n');
                Serial.print("Received: ");
                Serial.println(message);

                // Send an "ACK" response
                client.write("ACK\n");
                Serial.println("Sent ACK!");
            }
        }

        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
    }
}