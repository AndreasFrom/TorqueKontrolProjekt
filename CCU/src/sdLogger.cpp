#include "sdLogger.h"

SDLogger::SDLogger(const int spi_cs) {
    if(!SD.begin(spi_cs)){
        Serial.print("Could not init SD card!");
    }else{
        Serial.print("SD card init!");
    }
}

void SDLogger::init(const char *filename) {
    _filename = filename;
    _dataFile = SD.open(_filename, FILE_WRITE);
    if(_dataFile){
        _dataFile.println("timestamp, acc_x, acc_y, gyro_z");
        _dataFile.close();
    }else{
        Serial.print("Error opening file SD (Init)");
    }
}

void SDLogger::addData(const dataBlock& data) {
    _dataFile = SD.open(_filename, FILE_WRITE);
    if(_dataFile){
        String line = 
            String(data.timestamp) + ", " +
            String(data.acc_x) + ", " +
            String(data.acc_y) + ", " + 
            String(data.gyro_z);
        _dataFile.println(line);
        _dataFile.close();
    }else{
        Serial.print("Error opening file SD (Add)");
    }
}

bool SDLogger::getData(dataBlock &data) {
    if(!_fileOpenForRead){ // Check if file is already open
        _dataFile = SD.open(_filename, FILE_READ);
        if(!_dataFile){
            Serial.print("Error opening file SD (Get)");
            return false;
        }
        _fileOpenForRead = true;
        
        // Skip the header line
        if (_dataFile.available()) {
            _dataFile.readStringUntil('\n');
        }
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
        Serial.print("Error opening file SD (Get)");
        return false;
    }
}
