#include <Arduino.h>

void setup() {
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
}

void loop() {
  delay(1000);
  digitalWrite(18, HIGH);
  digitalWrite(19, HIGH);
  delay(1000);
  digitalWrite(18, LOW);
  digitalWrite(19, LOW); 
}

