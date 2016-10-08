

void setup() {
  for (int i=5; i < 15; ++i)
    pinMode(i, OUTPUT);
}

void loop() {

  for (int i=5; i < 15; ++i)
    digitalWrite(i, LOW);

  delay(1000);
  
  for (int i=5; i < 15; ++i)
    digitalWrite(i, HIGH);

  delay(1000);

}
