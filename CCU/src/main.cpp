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
#include "torqueControl.h"
#include "ICO_algo.h"
#include "math.h"
#include "filter.h"
#include <vector>


#define SEND_DATA_SERIAL false
#define AUTO_STOP_TIME 20 // seconds

const double SAMPLE_FREQ = 75.0; //100Hz, 10ms sample time
// WiFi Config
//WiFiHandler wifiHandler("coolguys123", "werty123", 4242);
//WiFiHandler wifiHandler("net", "simsimbims", 4242);
//WiFiHandler wifiHandler("Bimso", "banjomus", 4242);
WiFiHandler wifiHandler("Bimso", "banjomus", 4242);
WiFiClient client;

// ICO algorithms
ExponentialDecayFilter filter(0.1); // Example filter with alpha = 0.7
ExponentialDecayFilter filter1(0.5); // Example filter with alpha = 0.7
ExponentialDecayFilter filter2(0.8); // Example filter with alpha = 0.7
FIRFilter firFilter({0.1, 0.2, 0.3}); // Example FIR filter with coefficients

// ICO parameters
double omega0 = 0.2;
double omega1 = 0.4;
double eta = 0.0001;

// Create predictive vectors and populate immediately
std::vector<Predictive> predictive_vector_yaw = {
    Predictive(eta, omega1, new FIRFilter({-0.014, -0.127, -0.127, 0.111, 0.383, 0.383, 0.111, -0.127, -0.127, -0.014}))
};
std::vector<Predictive> predictive_vector_move = {
    Predictive(eta, omega1, new PassThroughFilter())
};

// Reflexes
Reflex reflex_yaw(omega0, 1.0 / SAMPLE_FREQ, new PIDFilter(1.24f, 5.27f, 0.0f, 1/SAMPLE_FREQ));
Reflex reflex_move(omega1, 1.0 / SAMPLE_FREQ, new PassThroughFilter());

// ICO algorithms
ICOAlgo ico_yaw(eta, 1.0 / SAMPLE_FREQ, reflex_yaw, predictive_vector_yaw);
ICOAlgo ico_move(eta, 1.0 / SAMPLE_FREQ, reflex_move, predictive_vector_move);



// SD card
const int chipselect = 10;
SDLogger sdLogger;

// I2C
#define SLAVE_ADDRESS_START 0x08 // Første I2C slaveadresse
I2CMaster i2cMaster;

// IMU
DFRobot_BMX160 bmx160;
sBmx160SensorData_t Ogyro_offset = {0, 0, 0};  
sBmx160SensorData_t Oaccel_offset = {0, 0, 0}; 

bool is_active = false; // Flag til logging


double start_time = 0;

// Temp data
float kp = 1.0;
float ki = 10.0;
float kd = 0.01;
uint8_t mode = 2;                      
float setpoint = 0;                  
float setpoint_radius = 0.5; 

double actual_velocity = 0; // Initialize actual velocity
double actual_velocity_x = 0; // Initialize actual velocity in x direction
double actual_velocity_y = 0; // Initialize actual velocity in y direction

double logging_time_start = 0; // Initialize logging time

double setpoint_yaw_degs = 0;
double updated_velocity = 0;
double error_yaw = 0;
double error_velocity = 0;

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

// Torque control
TorqueControl torque_control;
Currents currents; // Struct to hold currents for each wheel

// Prototypes
void handleClientCommunication(WiFiClient &client);
void processClientMessage(String message);
void calbrateIMU(void);

void timerISR() {
    if ((millis() - logging_time_start) >= (1000*AUTO_STOP_TIME) && is_active == true)
    {
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START,   0);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+1, 0);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+2, 0);
        i2cMaster.sendSetpoint(SLAVE_ADDRESS_START+3, 0);

        sdLogger.close();
        is_active = false;
        
        // Reset all setpoints
        

        //Serial.println("Logging stopped!");
    }

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
        Oaccel.x -= Oaccel_offset.x; // Offset for accelerometer
        Oaccel.y -= Oaccel_offset.y; // Offset for accelerometer
        Oaccel.z -= Oaccel_offset.z; // Offset for accelerometer
        //Ogyro.x -= Ogyro_offset.x; // Offset for gyroscope
        //Ogyro.y -= Ogyro_offset.y; // Offset for gyroscope
        //Ogyro.z -= Ogyro_offset.z; // Offset for gyroscope

        // Apply Kalman filtering
        float filtered_gyro_z = gyroFilterZ.updateEstimate(Ogyro.z) * 4;
        float filtered_accel_x = accelFilterX.updateEstimate(Oaccel.x);
        float filtered_accel_y = accelFilterY.updateEstimate(Oaccel.y);
        
        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Filtered Gyro Z: "); Serial.print(filtered_gyro_z); Serial.println(" °/s, ");
        Serial.print("Filtered Accel X: "); Serial.print(filtered_accel_x); Serial.println(" m/s², ");
        Serial.print("Filtered Accel Y: "); Serial.print(filtered_accel_y); Serial.println(" m/s²");
        #endif

        actual_velocity_x += filtered_accel_x * (1.0 / SAMPLE_FREQ);
        actual_velocity_y += filtered_accel_y * (1.0 / SAMPLE_FREQ);

        actual_velocity = sqrt(actual_velocity_x * actual_velocity_x + actual_velocity_y * actual_velocity_y); // Calculate the magnitude of the velocity vector

        double setpoint_yaw_degs = (setpoint / setpoint_radius) * 180/PI;
        
        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Setpoint: "); Serial.print(setpoint); Serial.println(" m/s, ");
        Serial.print("Setpoint Yaw: "); Serial.print(setpoint_yaw); Serial.println(" deg/s, ");
        Serial.print("Setpoint Radius: "); Serial.print(setpoint_radius); Serial.println(" m");
        #endif
        
        //double error_yaw = setpoint_yaw - filtered_gyro_z; // Used for datalogging
        error_yaw = ico_yaw.getError(); // Used for datalogging
        error_velocity = setpoint - actual_velocity;

        double yaw_input = constrain(filtered_gyro_z, 0, 500);
        //double updated_yaw = ico_yaw.computeChange(yaw_input, yaw_input , setpoint_yaw_degs);
        //double updated_system = ico_system.computeChange(updated_yaw,yaw_input, setpoint_yaw_degs);
        //updated_yaw = constrain(updated_yaw, 0, 3000); // Constrain updated_yaw between 0 and 300 deg/s
        //updated_yaw = constrain(updated_yaw, 0, 3000); // Constrain updated_yaw between 0 and 230 deg/s
        //double updated_velocity = ico_move.computeChange(actual_velocity, setpoint);
        double updated_yaw = 1;
        updated_velocity = ico_yaw.computeChange(yaw_input, yaw_input, setpoint_yaw_degs); // Constrain updated_velocity between 0 and 300 deg/s

        #ifdef SEND_DATA_CONTROL_SERIAL
        Serial.print("Updated Yaw: "); Serial.print(updated_yaw); Serial.println(" deg/s, ");
        Serial.print("Updated Velocity: "); Serial.print(updated_velocity); Serial.println(" m/s");
        #endif

        switch (mode) {
            case 0: {//   Velocity          
                // If setpoint is velocity
                
        
                //kinematic_model.getVelocities_acker_omega(updated_velocity, updated_yaw, Wheel_velocities);
                kinematic_model.getVelocities_acker(updated_velocity, 0.5, Wheel_velocities); // 0.5 radius of circle
                
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
                break; 
            }
            case 1: {// Torque
                torque_control.calculateCurrents(updated_velocity, currents);

                double motor_constant = 98.1;
                
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, currents.current_left_front * motor_constant);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 1, currents.current_right_front * motor_constant);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 2, currents.current_left_rear * motor_constant);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 3, currents.current_right_rear * motor_constant);
                break;
            }
            case 2: {// RPM
                // If setpoint is RPM
                kinematic_model.getVelocities_acker(setpoint, setpoint_radius, wheel_RPMs);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, wheel_RPMs.v_left_front);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 1, wheel_RPMs.v_right_front);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 2, wheel_RPMs.v_left_rear);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 3, wheel_RPMs.v_right_rear); 
                break; 
            }
            case 3: { // Disable ICO algorithms and use velocity control
                // If setpoint is velocity, set pid reflex, if reflex filter is PID
                
                kinematic_model.getVelocities_acker(setpoint, 0.5, Wheel_velocities); // 0.5 radius of circle

                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, Wheel_velocities.v_left_front);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 1, Wheel_velocities.v_right_front);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 2, Wheel_velocities.v_left_rear);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 3, Wheel_velocities.v_right_rear);
                break; 
            }
            case 4: { // Disable ICO algorithms and use torque control
                // If setpoint is torque, set pid reflex
                torque_control.calculateCurrents(setpoint, currents);

                double motor_constant = 98.1;
                
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START, currents.current_left_front * motor_constant);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 1, currents.current_right_front * motor_constant);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 2, currents.current_left_rear * motor_constant);
                i2cMaster.sendSetpoint(SLAVE_ADDRESS_START + 3, currents.current_right_rear * motor_constant);
                break; 
                break;
            }
            default:
                //do nothing
                break;
        }
        
        
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
            static_cast<float>(ico_yaw.getOmega1()), 
            static_cast<float>(ico_yaw.getPredictiveSum()),
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

    //predictive_vector_yaw.emplace_back(eta, omega1, &filter2);
    //predictive_vector_move.emplace_back(eta, omega1, &filter2);

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
        ico_move.clearFilters(); // Reset filters for ICO
        ico_yaw.clearFilters(); // Reset filters for ICO
        is_active = true;
        Serial.println("Logging started!");
        logging_time_start = millis(); // Start logging time

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

        calbrateIMU(); // Calibrate IMU after stopping logging
        Serial.println("IMU calibrated!");
        
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

        int mu_mode = 0;
        if (mode <= 2)
            mu_mode = mode;
        else if (mode == 3) //If 3 disable, ico algorithms and use velocity control
            mu_mode = 0;
        else if (mode == 4) //If 4 disable, ico algorithms and use Torque control
            mu_mode = 1;

        for (int i = 0; i < 4; i++) {
            bool success = i2cMaster.sendParam(SLAVE_ADDRESS_START + i, mu_mode, kp, ki, kd);
            if (!success) {
                Serial.println("I2C communication failed!");
            }
        }
        
        if (mode == 0) {
            // If mode is velocity, set pid reflex, if reflex filter is PID
            if (reflex_move.getFilter()->getType() == "PID") {
                static_cast<PIDFilter*>(reflex_move.getFilter())->setParameters(1.24f, 5.27f, 0.0f);
            } else {
                Serial.println("Reflex filter is not PID, no PID reflex set.");
                return;
            }
        } else if (mode == 1) {
            // If mode is torque, set pid reflex
            if (reflex_yaw.getFilter()->getType() == "PID") {
                static_cast<PIDFilter*>(reflex_yaw.getFilter())->setParameters(1.24f, 5.27f, 0.0f); // 19.35f, 45.98f, 0.0f Legacy
            } else {
                Serial.println("Reflex filter is not PID, no PID reflex set.");
                return;
            }
        } else {
            Serial.println("Mode not 0 or 1, no PID reflex set.");
            return;
        }

        kinematic_model.getVelocities_acker(setpoint, setpoint_radius, wheel_RPMs);
        Serial.print("M0: "); Serial.println(wheel_RPMs.v_left_front);
        Serial.print("M1: "); Serial.println(wheel_RPMs.v_right_front);
        Serial.print("M2: "); Serial.println(wheel_RPMs.v_left_rear);
        Serial.print("M3: "); Serial.println(wheel_RPMs.v_right_rear);


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
    } else if (message.startsWith("ICO:")){ //Format to recieve: Received: ICO:0.5,0.9,0.0001
        client.println("ACK:ICO");
        message.remove(0, 4);
        int comma0 = message.indexOf(',');
        int comma1 = message.indexOf(',', comma0 + 1);

        if (comma0 == -1 || comma1 == -1) {
            Serial.println("Error: Invalid ICO format. Expected 'ICO:omega0,omega1'");
            return;
        }
        
        omega0 = message.substring(0, comma0).toFloat();
        omega1 = message.substring(comma0 + 1).toFloat();
        eta = message.substring(comma1 + 1).toFloat();
        
        if (omega1 <= omega0) {
            Serial.println("Warning: omega1 should be greater than omega0");
        }
        
        Serial.print("Updated ICO parameters - omega0: "); 
        Serial.print(omega0);
        Serial.print(", omega1: "); 
        Serial.println(omega1);
        Serial.print("Updated ICO eta: ");
        Serial.println(eta);

        if (is_active) {
            Serial.println("Warning: Updating ICO parameters during active operation");
        }
        
        ico_yaw.updateOmegaValues(omega0, omega1); 
        ico_move.updateOmegaValues(omega0, omega1);
        ico_yaw.setEta(eta);
        ico_move.setEta(eta);
        ico_yaw.resetICO();
        ico_move.resetICO();
    }
}

void calbrateIMU(void)
{// If not active, calibrate the IMU
    sBmx160SensorData_t Ogyro = {0, 0, 0};  
    sBmx160SensorData_t Oaccel = {0, 0, 0}; 
    sBmx160SensorData_t Omagn = {0, 0, 0}; 

    float gyro_x_sum = 0;
    float gyro_y_sum = 0;
    float gyro_z_sum = 0;

    float accel_x_sum = 0;
    float accel_y_sum = 0;
    float accel_z_sum = 0;

    for (int i = 0; i < 100; i++) {
        bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
        gyro_x_sum += Ogyro.x;
        gyro_y_sum += Ogyro.y;
        gyro_z_sum += Ogyro.z;

        accel_x_sum += Oaccel.x;
        accel_y_sum += Oaccel.y;
        accel_z_sum += Oaccel.z;

        delay(5); // Wait for 5ms between samples
    }
    double gyro_x_offset = gyro_x_sum / 100;
    double gyro_y_offset = gyro_y_sum / 100;
    double gyro_z_offset = gyro_z_sum / 100;
    double accel_x_offset = accel_x_sum / 100;
    double accel_y_offset = accel_y_sum / 100;
    double accel_z_offset = accel_z_sum / 100;

    Ogyro_offset = {gyro_x_offset, gyro_y_offset, gyro_z_offset};
    Oaccel_offset = {accel_x_offset, accel_y_offset, 9.82 - accel_z_offset};
}