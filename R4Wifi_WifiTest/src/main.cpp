#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>

/* WiFi network name and password */
const char* ssid = "coolguys123";
const char* pwd = "werty123";

// IP address of your PC running the Python script
const char* udpAddress = "192.168.93.49";  // Updated to your actual PC IP
const int udpPort = 4242;

#define I2C_SDA 21
#define I2C_SCL 22
#define LSM6DSO_ADDRESS 0xD5

#define DEV_I2C Wire

// Create UDP instance
WiFiUDP udp;

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for Serial to be ready

    // Connect to WiFi
    WiFi.begin(ssid, pwd);
    Serial.print("Connecting to WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start UDP
    udp.begin(udpPort);
}

void loop() {
    int32_t accelerometer[3] = {0, 1, 2};  // Replace with actual accelerometer data

    char data_string[50];
    snprintf(data_string, sizeof(data_string), "%ld,%ld,%ld", 
             accelerometer[0], accelerometer[1], accelerometer[2]);

    bool ack_received = false;
    int retry_count = 0;
    const int max_retries = 3;

    while (!ack_received && retry_count < max_retries) {
        Serial.print("Sending data: ");
        Serial.println(data_string);

        // Send UDP packet
        udp.beginPacket(udpAddress, udpPort);
        udp.write(data_string);
        udp.endPacket();

        unsigned long start_time = millis();
        char ack_buffer[10];

        // Wait for ACK (1-second timeout)
        while (millis() - start_time < 1000) {
            int packet_size = udp.parsePacket();
            if (packet_size) {
                int len = udp.read(ack_buffer, sizeof(ack_buffer) - 1);
                if (len > 0) {
                    ack_buffer[len] = '\0';
                    if (strcmp(ack_buffer, "ACK") == 0) {
                        ack_received = true;
                        Serial.println("ACK received!");
                        break;
                    }
                }
            }
        }

        if (!ack_received) {
            retry_count++;
            Serial.println("ACK not received, retrying...");
        }
    }

    if (!ack_received) {
        Serial.println("Failed to receive ACK after multiple attempts.");
    }

    delay(1000);  // Wait before sending the next packet
}
