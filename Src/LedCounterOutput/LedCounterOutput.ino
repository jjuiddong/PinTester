
//int led[10] = {14, 13, 10, 11, 12, 5, 6, 7, 9, 8 };
int led[10] = {8, 9, 7, 6, 5, 12, 11, 10, 13, 14};

void setup() {
  for (int i=5; i < 15; ++i)
    pinMode(i, INPUT_PULLUP);
}

int count = 0;
void loop() {
  
}
