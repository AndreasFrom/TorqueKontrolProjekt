#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <WiFiS3.h>  // Change from <WiFi.h> to <WiFiS3.h>

class WiFiHandler {
private:
    const char* ssid;      
    const char* password;  
    int tcpPort;           
    WiFiServer server;     

public:
    WiFiHandler(const char* ssid, const char* password, int tcpPort);
    void connectToWiFi();
    void startTCPServer();
    WiFiClient acceptClient();
};

#endif
