#include "wifihandler.h"

// Constructor
WiFiHandler::WiFiHandler(const char* ssid, const char* password, int tcpPort)
    : ssid(ssid), password(password), tcpPort(tcpPort), server(tcpPort) {}

// Connect to Wi-Fi
void WiFiHandler::connectToWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// Start the TCP server
void WiFiHandler::startTCPServer() {
    server.begin();
    Serial.println("TCP Server is listening...");
}

// Check for client connections
WiFiClient WiFiHandler::acceptClient() {
    return server.available();
}