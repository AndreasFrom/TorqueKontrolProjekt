#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <WiFi.h>

class WiFiHandler {
private:
    const char* ssid;      // Wi-Fi SSID
    const char* password;  // Wi-Fi password
    int tcpPort;           // TCP port for the server
    WiFiServer server;     // TCP server

public:

    WiFiHandler(const char* ssid, const char* password, int tcpPort);
    void connectToWiFi();
    void startTCPServer();
    WiFiClient acceptClient();
};

#endif