#include "sdLogger.h"

SDLogger::SDLogger(void) {
}

void SDLogger::init(const int spi_cs, const char *filename) {
    if(!SD.begin(spi_cs)){
        Serial.println("Could not init SD card!");
    }else{
        Serial.println("SD card init!");
    }
    _filename = filename;
    _dataFile = SD.open(_filename, FILE_WRITE);
    if(_dataFile){
        _dataFile.close();
        Serial.println("SD init complete");
    }else{
        Serial.println("Error opening file SD (Init)");
    }
}

void SDLogger::addData(const dataBlock& data) {
    if(!_fileOpen){ // Check if file is already open
        _dataFile = SD.open(_filename, FILE_WRITE);
        _fileOpen = true;
        _dataFile.println(_dataHeader);
    }

    if(_dataFile){
        String line = 
            String(data.timestamp) + ", " +
            String(data.mode) + ", " +
            String(data.setpoint) + ", " +
            String(data.setpoint_radius) + ", " +
            String(data.acc_x) + ", " +
            String(data.acc_y) + ", " + 
            String(data.gyro_z) + ", " +
            String(data.Kp) + ", " +
            String(data.Ki) + ", " +
            String(data.Kd) + ", " +
            String(data.MU0.setpoint_recv) + ", " +
            String(data.MU0.value_recv) + ", " +
            String(data.MU0.current_recv) + ", " +
            String(data.MU1.setpoint_recv) + ", " +
            String(data.MU1.value_recv) + ", " +
            String(data.MU1.current_recv) + ", " +
            String(data.MU2.setpoint_recv) + ", " +
            String(data.MU2.value_recv) + ", " +
            String(data.MU2.current_recv) + ", " +
            String(data.MU3.setpoint_recv) + ", " +
            String(data.MU3.value_recv) + ", " +
            String(data.MU3.current_recv) + ", " +
            String(data.error_yaw) + ", " +
            String(data.error_velocity) + ", " +
            String(data.updated_yaw) + ", " +
            String(data.updated_velocity);
        _dataFile.println(line);
        //Serial.println(line);
    }else{
        Serial.println("Error opening file SD (Add)");
    }
}

void SDLogger::close() {
    if(_fileOpen){
        _dataFile.close();
        _fileOpen = false;
    }
}
