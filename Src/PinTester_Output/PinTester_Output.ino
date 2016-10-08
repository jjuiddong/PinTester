//
// 2016-10-07, pin tester board (output board)
//
#include <SoftwareSerial.h>

// recv protocol
//  - C + board number(1,2,3) + CRC
//  - I + Led On/Off (1,0) + CRC
//  - P + pin number(0 ~ 19) + CRC
//  - L + pin number(0 ~ 19) + CRC
//  - B + 0 + CRC

int boardNumber = 1; // 0:master input board, 1:input board2, 2:output board3, 3:output board4
int state = 0; // 0:receive mode, 1,2:blink mode
int readBytes = 0;
char readBuffer[8];
SoftwareSerial mySerial(4, (boardNumber == 1) ? 2 : 3); // RX, TX

int board_led[20] = {10, 11, 12, 13, 14, 8, 9, 7, 6, 5, 
                      8, 9, 7, 6, 5, 12, 11, 10, 13, 14 };

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.print("Board");
  Serial.print(boardNumber + 1);
  Serial.println(" On");

  if (1 == boardNumber)
  {
    for (int i = 0; i < 10; ++i)
    {
      pinMode(board_led[i], OUTPUT);
      digitalWrite(board_led[i], LOW);
    }
  }
  else
  {
    for (int i = 0; i < 10; ++i)
    {
      //pinMode(board_led[i], INPUT);
      pinMode(board_led[i], OUTPUT);
      digitalWrite(board_led[i], HIGH);
    }
  }

  delay(100);  
}

void CriticalErrorSend(int type)
{
  if (boardNumber == 1) // only board2
  {
    char buffer[4];
    buffer[0] = 'E';
    buffer[1] = '1' + boardNumber;
    buffer[2] = '1' + type - 1;
    buffer[3] = (char)(((int)buffer[0] + (int)buffer[1] + (int)buffer[2]) % 256);
    mySerial.write(buffer, 4);
  }

  Serial.print("Critical error send type=");
  Serial.println(type);
}

void PrintReadBuffer()
{
  Serial.print(readBuffer[0]);
  Serial.print(readBuffer[1]);
  Serial.println(readBuffer[2]);
}

void PinTest(int pin)
{
  if ( ((2 == boardNumber) && (pin >= 10) && (pin <= 19)) || // board3
        ((3 == boardNumber) && (pin >= 0) && (pin <= 9)) )
  {
    char buffer[4];
    buffer[0] = 'P';
    buffer[1] = pin;
    if (digitalRead(board_led[pin]) == LOW)
    {
      buffer[2] = 'S';  
      Serial.print("Success Pin Test ");
      Serial.println(pin);
    }
    else
    {
      buffer[2] = 'F';
      Serial.print("Fail Pin Test ");
      Serial.println(pin);
    }
    buffer[3] = (char)(((int)buffer[0] + (int)buffer[1] + (int)buffer[2]) % 256);
    
    mySerial.write(buffer, 4);
  }
}

void loop()
{
  if (!mySerial.available())
  {
    if (1 == state)
    {
      state = 2;
      for (int i = 0; i < 10; ++i)
      {
        digitalWrite(board_led[i], LOW);
      }
      delay(500);
    }
    else if (2 == state)
    {
      state = 1;
      for (int i = 0; i < 10; ++i)
      {
        digitalWrite(board_led[i], HIGH);
      }
      delay(500);
    }
    
    return;
  }
    
  readBytes += mySerial.readBytes(&readBuffer[readBytes], 3 - readBytes);
  if (readBytes < 3)
    return;
    
  // check Header
  if ( (readBuffer[0] != 'C') 
  && (readBuffer[0] != 'I') 
  && (readBuffer[0] != 'P')
  && (readBuffer[0] != 'L')
  && (readBuffer[0] != 'B')
  )
  {
    PrintReadBuffer();
  
    // buffer rotation
    readBuffer[0] = readBuffer[1];
    readBuffer[1] = readBuffer[2];
    readBytes = 2;
  
    Serial.println("Error Header");
    return;
  }
  
  readBytes = 0;
  
  // check CRC
  char crc = (char)(((int)readBuffer[0] + (int)readBuffer[1]) % 256);
  if (crc != readBuffer[2])
  {
    PrintReadBuffer();
    
    CriticalErrorSend(1);
    Serial.println("Error Check CRC");
    return;
  }
  
  char buffer[4];  
  if ('C' == readBuffer[0]) // check board
  {
    state = 0;
    int bnum = (int)(readBuffer[1] - '1'); // board number
    if (bnum == boardNumber)
    {      
      buffer[0] = 'C';
      buffer[1] = '1' + boardNumber;
      buffer[2] = 'S';
      buffer[3] = (char)(((int)buffer[0] + (int)buffer[1] + (int)buffer[2]) % 256);
      mySerial.write(buffer, 4);
      Serial.print("Success Check slave board number ");
      Serial.println( readBuffer[1] );
    }
  }
  else if ('I' == readBuffer[0]) // initialize
  {
    state = 0;
    Serial.print("Initialize ");
    Serial.println(readBuffer[1]);
    
    if (1 == boardNumber) // input board
    {
        // led off
        for (int i=0; i < 10; ++i)
        {
          digitalWrite(board_led[i], (readBuffer[1]=='0')? HIGH : LOW );    
        }
    }
    else // output board
    {
      for (int i = 0; i < 10; ++i)
      {
        pinMode(board_led[i], INPUT);
      }
    }
  }
  else if ('P' == readBuffer[0])
  {
    state = 0;
    int pinNumber = (int)readBuffer[1];
    Serial.print("PinTest ");
    Serial.println((int)readBuffer[1]);
    
    if (1 == boardNumber) // input board
    {
      if ((pinNumber >= 0) && (pinNumber <= 9))
      {
        // board2 pin test
        digitalWrite(board_led[pinNumber], LOW);
      }
    }
    else // output board
    {
      delay(100);
      PinTest(pinNumber);
    }    
  }
  else if ('L' == readBuffer[0])
  {
    state = 0;
    int pinNumber = (int)readBuffer[1];
    Serial.print("LED On ");
    Serial.println(pinNumber);
    
    if (1 == boardNumber) // input board
    {
      if ((pinNumber >= 0) && (pinNumber <= 9))
      {
        // board2 led on
        pinMode(board_led[pinNumber], OUTPUT);
        digitalWrite(board_led[pinNumber], LOW);
      }
    }
    else
    {
    }
  }  
  else if ('B' == readBuffer[0]) // blink
  {
    Serial.println("Blink");
    
    if (1 == boardNumber) // input board
    {
      state = 1;
    }
  }  
  
}

