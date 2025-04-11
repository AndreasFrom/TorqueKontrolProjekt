#include <Arduino.h>
#include <Wire.h>
#include <queue>
#include <SD.h>
#include "AGTimerR4.h" //https://github.com/washiyamagiken/AGTimer_R4_Library/tree/main
#include "wifihandler.h" 
#include <DFRobot_BMX160.h>
#include "sdLogger.h"
#include "i2c_master.h"
#include "SimpleKalmanFilter.h"
#include "kinematic.h"
#include "ICO_algo.h"
#include "math.h"


#define SEND_DATA_SERIAL false
// WiFi Config
//WiFiHandler wifiHandler("coolguys123", "werty123", 4242);
//WiFiHandler wifiHandler("net", "simsimbims", 4242);
WiFiHandler wifiHandler("Bimso", "banjomus", 4242);
//WiFiHandler wifiHandler("ANDREASPC", "banjomus", 4242);
WiFiClient client;
ICOAlgo ico_yaw(0.0001,2,0.01);
ICOAlgo ico_move(0.0001,1,0.01);

// SD card
const int chipselect = 10;
SDLogger sdLogger;

// I2C
#define SLAVE_ADDRESS_START 0x08 // Første I2C slaveadresse
I2CMaster i2cMaster;

// IMU
DFRobot_BMX160 bmx160;

bool is_active = false; // Flag til logging
const double SAMPLE_FREQ = 100.0; //100Hz, 10ms sample time

double start_time = 0;

// Temp data
float kp = 1.0;
float ki = 10.0;
float kd = 0.01;
uint8_t mode = 2;                      
float setpoint = 0;                  
float setpoint_radius = 0.5; 

double actual_velocity = 0; // Initialize actual velocity

// int setpoint0 = 711; // left front
// int setpoint1 = 916.9; // right front
// int setpoint2 = 582.8; // left rear
// int setpoint3 = 821.5; // right rear

// Create Kalman filters for each axis with appropriate parameters
// Gyroscope: 0.07 °/s noise
SimpleKalmanFilter gyroFilterX(0.07, 1.0, 0.01);
SimpleKalmanFilter gyroFilterY(0.07, 1.0, 0.01);
SimpleKalmanFilter gyroFilterZ(0.07, 1.0, 0.01);

// Accelerometer: 1.8mg noise (0.01766 m/s²)
SimpleKalmanFilter accelFilterX(0.01766, 1.0, 0.01);
SimpleKalmanFilter accelFilterY(0.01766, 1.0, 0.01);
SimpleKalmanFilter accelFilterZ(0.01766, 1.0, 0.01);

Kinematic kinematic_model;
Velocities_acker wheel_RPMs; // Struct to hold wheel velocities
Velocities_acker Wheel_velocities; // Struct to hold wheel velocities

// Prototypes
void handleClientCommunication(WiFiClient &client);
void processClientMessage(String message);

void timerISR() {
    if(is_active){
        //Perform measurements and add to queue
        unsigned long timestamp = millis();   

        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Timestamp: "); Serial.print(timestamp); Serial.println(" ms, ");
        #endif
        

        sBmx160SensorData_t Ogyro = {0, 0, 0};  
        sBmx160SensorData_t Oaccel = {0, 0, 0}; 
        sBmx160SensorData_t Omagn = {0, 0, 0};  
        //bmx160.getGyroACC(&Ogyro, &Oaccel);
        bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
        //sdLogger.addData({timestamp, Oaccel.x, Oaccel.y, Oaccel.z});

        // Apply Kalman filtering
        float filtered_gyro_z = gyroFilterZ.updateEstimate(Ogyro.z) * 4;
        float filtered_accel_x = accelFilterX.updateEstimate(Oaccel.x);
        float filtered_accel_y = accelFilterY.updateEstimate(Oaccel.y);
        
        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Filtered Gyro Z: "); Serial.print(filtered_gyro_z); Serial.println(" °/s, ");
        Serial.print("Filtered Accel X: "); Serial.print(filtered_accel_x); Serial.println(" m/s², ");
        Serial.print("Filtered Accel Y: "); Serial.print(filtered_accel_y); Serial.println(" m/s²");
        #endif

        
        actual_velocity += filtered_accel_y * (1.0 / SAMPLE_FREQ); // Integrate acceleration over time

        // If setpoint is velocity
        double setpoint_yaw_degs = (setpoint / setpoint_radius) * 180/PI;

        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Setpoint: "); Serial.print(setpoint); Serial.println(" m/s, ");
        Serial.print("Setpoint Yaw: "); Serial.print(setpoint_yaw); Serial.println(" deg/s, ");
        Serial.print("Setpoint Radius: "); Serial.print(setpoint_radius); Serial.println(" m");
        #endif
        
        //double error_yaw = setpoint_yaw - filtered_gyro_z; // Used for datalogging
        double error_yaw = ico_yaw.getError(); // Used for datalogging
        double error_velocity = setpoint - actual_velocity;

        double updated_yaw = ico_yaw.computeChange(constrain(filtered_gyro_z, 0, 500), setpoint_yaw_degs);
        updated_yaw = constrain(updated_yaw, 20, 230); // Constrain updated_yaw between 0 and 230 deg/s
        //double updated_velocity = ico_move.computeChange(actual_velocity, setpoint);
        double updated_velocity = setpoint; 

        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Updated Yaw: "); Serial.print(updated_yaw); Serial.println(" deg/s, ");
        Serial.print("Updated Velocity: "); Serial.print(updated_velocity); Serial.println(" m/s");
        #endif

        kinematic_model.getVelocities_acker_omega(updated_velocity, updated_yaw, Wheel_velocities);

        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Wheel Velocities: ");
        Serial.print("Left Front: "); Serial.print(Wheel_velocities.v_left_front); Serial.println(" m/s, ");
        Serial.print("Right Front: "); Serial.print(Wheel_velocities.v_right_front); Serial.println(" m/s, ");
        Serial.print("Left Rear: "); Serial.print(Wheel_velocities.v_left_rear); Serial.println(" m/s, ");
        Serial.print("Right Rear: "); Serial.print(Wheel_velocities.v_right_rear); Serial.println(" m/s");
        #endif

        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, Wheel_velocities.v_left_front);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 1, Wheel_velocities.v_right_front);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 2, Wheel_velocities.v_left_rear);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 3, Wheel_velocities.v_right_rear);

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
            filtered_accel_x, filtered_accel_y, filtered_gyro_z,
            actual_velocity,
            kp, ki, kd,
            MU0, MU1, MU2, MU3,
            error_yaw, error_velocity,
            updated_yaw, updated_velocity,
            ico_yaw.getOmega1(), ico_move.getOmega1()
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
    delay(1000); // Wait for SD card to initialize

    // Initialize Timer1 to trigger every 10ms
    AGTimer.init(SAMPLE_FREQ, timerISR);
    AGTimer.start();

    if (!bmx160.begin()) {
        Serial.println("Sensor init fejlede!");
        while (1);
    }
    Serial.println("Setup complete!");
    bmx160.setGyroRange(eGyroRange_500DPS); // Gyro range
    bmx160.setAccelRange(eAccelRange_2G); // Accel range

    // Temp until send from commander works
 /*    i2cMaster.sendParam(SLAVE_ADDRESS_START, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, setpoint0);  
    i2cMaster.sendParam(SLAVE_ADDRESS_START+1, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+1, setpoint1);
    i2cMaster.sendParam(SLAVE_ADDRESS_START+2, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+2, setpoint2);
    i2cMaster.sendParam(SLAVE_ADDRESS_START+3, mode, kp, ki, kd);
    i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+3, setpoint3); */

    //start_time = millis();
}

void loop() {
    client = wifiHandler.acceptClient();
    if (client) {
        Serial.println("Client connected!");
        while (client.connected()) {
            handleClientCommunication(client);
        }
        client.stop();
        is_active = false; // Reset is_active flag when client disconnects
        Serial.println("Client disconnected.");
    }
/*     int time = millis() - start_time;
    if (time < 10000) { // Stop logging after 10 seconds
        logging = true;
    } else {
        logging = false;
        sdLogger.close();
        Serial.println("Logging stopped at: " + String(time));
    } */

    //delay(20); // 50Hz

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
        is_active = true;
        // Set all setpoints
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START,   wheel_RPMs.v_left_front);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+1, wheel_RPMs.v_right_front);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+2, wheel_RPMs.v_left_rear);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+3, wheel_RPMs.v_right_rear);
        Serial.println("Logging started!");

    } else if (message == "STOP") {
        client.println("ACK:STOP");
        is_active = false;
        sdLogger.close();
        // Reset all setpoints
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START,   0);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+1, 0);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+2, 0);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+3, 0);
        Serial.println("Logging stopped!");
        
    } else if (message.startsWith("PID:")) {
        client.println("ACK:PID");
        message.remove(0, 4);
        int comma1 = message.indexOf(',');
        int comma2 = message.indexOf(',', comma1 + 1);
        int comma3 = message.indexOf(',', comma2 + 1);
        int comma4 = message.indexOf(',', comma3 + 1);

        if (comma1 == -1 || comma2 == -1 || comma3 == -1 || comma4 == -1) {
            Serial.println("Fejl: Forkert PID-format");
            return;
        }

        kp = message.substring(0, comma1).toFloat();
        ki = message.substring(comma1 + 1, comma2).toFloat();
        kd = message.substring(comma2 + 1, comma3).toFloat();
        setpoint = message.substring(comma3 + 1, comma4).toFloat();
        mode = message.substring(comma4 + 1).toInt();

        Serial.print("Parsed PID: ");
        Serial.print("Setpoint: "); Serial.print(setpoint);
        Serial.print(" Mode: "); Serial.print(mode);
        Serial.print(" Kp: "); Serial.print(kp);
        Serial.print(" Ki: "); Serial.print(ki);
        Serial.print(" Kd: "); Serial.println(kd);

        for (int i = 0; i < 4; i++) {
            bool success = i2cMaster.sendParam(SLAVE_ADDRESS_START + i, mode, kp, ki, kd);
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
            bool success = i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + i, 0);
            if (!success) {
                Serial.println("I2C communication failed!");
            }
        }
    }
}