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
            String(data.MU3.current_recv);
        _dataFile.println(line);
        Serial.println(line);
    }else{
        Serial.println("Error opening file SD (Add)");
    }
}

bool SDLogger::getData(dataBlock &data) {
    if(!_fileOpen){ // Check if file is already open
        _dataFile = SD.open(_filename, FILE_READ);

        if(!_dataFile){
            Serial.println("Error opening file SD (Get)");
            return false;
        }
        _fileOpen = true;
            
        // Skip the header line
        if (_dataFile.available()) {
            _dataFile.readStringUntil('\n');
        }
        Serial.println("Transmitting data...");
    }
    
    
    if(_dataFile.available()){
        String line = _dataFile.readStringUntil('\n');
        int index = 0;
        int nextIndex = 0;
        for(int i = 0; i < 4; i++){
            nextIndex = line.indexOf(',', index);
            switch(i){
                case 0:
                    data.timestamp = line.substring(index, nextIndex).toInt();
                    break;
                case 1:
                    data.acc_x = line.substring(index, nextIndex).toFloat();
                    break;
                case 2:
                    data.acc_y = line.substring(index, nextIndex).toFloat();
                    break;
                case 3:
                    data.gyro_z = line.substring(index, nextIndex).toFloat();
                    break;
            }
            index = nextIndex + 2;
        }
        return true;
    }else{
        _dataFile.close();
        _fileOpen = false;
        Serial.println("No more data avaliable");
        return false;
    }
}

void SDLogger::close() {
    if(_fileOpen){
        _dataFile.close();
        _fileOpen = false;
    }
}
