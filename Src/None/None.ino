
#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 3); // RX, TX


void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {

}
