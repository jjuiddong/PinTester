#include <SoftwareSerial.h>

int pot1Pin = 2;
int pot2Pin = 3;
SoftwareSerial mySerial(2, 3); // RX, TX

void setup() 
{
  mySerial.begin(9600);
}

void loop()
{
  mySerial.println("Hello, world?");
  delay(500);
}

