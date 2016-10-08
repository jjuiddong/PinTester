
int led[10] = {10, 11, 12, 13, 14, 8, 9, 7, 6, 5 };

void setup() {
  for (int i=5; i < 15; ++i)
    pinMode(i, OUTPUT);
}

int count = 0;
void loop() {

  for (int i=5; i < 15; ++i)
    digitalWrite(i, HIGH);

  digitalWrite(led[count], LOW);
  ++count;
  if (count >= 10)
    count = 0;

  delay(300);
}
