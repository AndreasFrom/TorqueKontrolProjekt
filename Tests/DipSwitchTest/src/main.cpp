#include <Arduino.h>

#define DIPSWITCH_1 7
#define DIPSWITCH_2 8

void setup() {
  pinMode(DIPSWITCH_1, INPUT);
  pinMode(DIPSWITCH_2, INPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.print("DIPSWITCH_1: ");
  Serial.println(digitalRead(DIPSWITCH_1));
  Serial.print("DIPSWITCH_2: ");
  Serial.println(digitalRead(DIPSWITCH_2));
  delay(1000);
} 