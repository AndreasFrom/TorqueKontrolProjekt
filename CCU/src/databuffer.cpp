#include "databuffer.h"

DataBuffer::DataBuffer() {
}

void DataBuffer::addData(const dataBlock& data) {
    if(_dataBuffer.size() >= _maxBufferSize){
        Serial.println("Buffer is full!");
    }else{
        _dataBuffer.push(data);
    }
}

bool DataBuffer::getData(dataBlock& data) {
    if(_dataBuffer.empty()){
        return false;
    }
    data = _dataBuffer.front();
    _dataBuffer.pop();
    return true;
}
