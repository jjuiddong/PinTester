//
// 2016-10-07, pin tester board (Master Input Board)
//
#include <SoftwareSerial.h>

int boardNumber = 0; // 0:master input board, 1:input board2, 2:output board3, 3:output board4
int state = 0; // 0:Led On, 1:Check board, 2:init fail, 3:initialize(Led Off), 4:pin test, 5:finish test, 6:finish, 7:blink
char initialBoard = 2; // 0:board1, 1:board2, 2:board3, 3:board4
int commMode = 0; // 0:send, 1:recv
int readBytes = 0;
char readBuffer[8];
SoftwareSerial mySerial(3, 4); // RX, TX

int pinNumber = 0;
int board_led[20] = {10, 11, 12, 13, 14, 8, 9, 7, 6, 5, 8, 9, 7, 6, 5, 12, 11, 10, 13, 14 };
bool led_check[20];
bool blinkType = true;

void setup() 
{
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.print("Board");
  Serial.print(boardNumber+1);
  Serial.println(" On");

  delay(100);
}

void PrintReadBuffer()
{
  Serial.print(readBuffer[0]);
  Serial.print((int)readBuffer[1]);
  Serial.print((int)readBuffer[2]);
  Serial.println(readBuffer[3]);
}


bool PacketCheck(char protocol)
{
  // Protocol
  // T + board number(1,2,3) + result(F:fail, S:success) + CRC
  
  if (!mySerial.available()) 
    return false;
    
  readBytes += mySerial.readBytes(&readBuffer[readBytes], 4-readBytes);
  if (readBytes < 4)
    return false;
    
  // check Header
  if ((readBuffer[0] != 'C') && (readBuffer[0] != 'E') && (readBuffer[0] != 'P'))
  {
    PrintReadBuffer();
    
    // buffer rotation
    readBuffer[0] = readBuffer[1];
    readBuffer[1] = readBuffer[2];
    readBuffer[2] = readBuffer[3];
    readBytes = 3;

    Serial.println("Error Header");
    return false;
  }
  
  readBytes = 0;
    
  // check CRC
  char crc = (char)(((int)readBuffer[0] + (int)readBuffer[1] + (int)readBuffer[2]) % 256);
  if (crc != readBuffer[3])
  {
    // error, re send
    PrintReadBuffer();
    Serial.println("Error Check CRC");
    return false;
  }

  if (readBuffer[0] != protocol)
  {
    // error, re send
    PrintReadBuffer();
    Serial.println("Error Protocol");
    return false;
  }

  return true;
}


void loop()
{
 
  if (0 == state)
  {
    // Board Initialize Protocol
    char buffer[3];
    buffer[0] = 'I';
    buffer[1] = '1'; // LED On
    buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
    mySerial.write(buffer, 3);
    Serial.println("Send Initialize Board '1'");

    for (int i=0; i < 10; ++i)
    {
      pinMode(board_led[i], OUTPUT);
      digitalWrite(board_led[i], LOW);
    }

    state = 1;
    commMode = 0;
  }
  else if (1 == state) // board check
  {
      if (commMode == 0) // send
      {
        // Board Check Protocol
        char buffer[3];
        buffer[0] = 'C';
        buffer[1] = '1' + initialBoard; // '2'-'3'-'4'
        buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
        mySerial.write(buffer, 3);

        Serial.print("Send Check Board");
        Serial.println(initialBoard + 1); // debug
        commMode = 1;
        readBytes = 0;
      }
      else // rcv
      {
        if (!PacketCheck('C'))
        {
          commMode = 0;
          return;
        }

        // check board number
        if ((readBuffer[1] != '2') && (readBuffer[1] != '3') && (readBuffer[1] != '4'))
        {
           commMode = 0;
           PrintReadBuffer();
           Serial.print("Error Check board number ");
           Serial.println( readBuffer[1] );
           return;
        }

        if (readBuffer[2] == 'F')
        {
           commMode = 0;
           Serial.print("Fail Check board number ");
           Serial.println(readBuffer[1] );
           return;
        }

        int bnum = (int)(readBuffer[1] - '1'); // board number
        if (bnum == 3)
        {
          // success Check board
          state = 3;
          commMode = 0;

          // LED Off
          delay(500);            
          for (int i=0; i < 10; ++i)
          {
            digitalWrite(board_led[i], HIGH);
          }
        }
        else
        {
          initialBoard = bnum + 1;
          commMode = 0;
        }
          
        Serial.print("Success Check board number ");
        Serial.println( readBuffer[1] );
      }
  }
  else if (2 == state) // init fail
  {
    
  }
  else if (3 == state) // initialize
  {
    // Board Initialize Protocol
    char buffer[3];
    buffer[0] = 'I';
    buffer[1] = '0'; // LED Off
    buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
    mySerial.write(buffer, 3);
    Serial.println("Send Initialize Board '0'");

    state = 4;
    commMode = 0;
  }
  else if (4 == state) // Pin Test
  {
    if (0 == commMode) // send mode
    {
      if (pinNumber >= 20)
      {
        state = 5; // Finish Pin Test
        return;
      }
      
      char buffer[3];
      buffer[0] = 'P';
      buffer[1] = pinNumber; // Test Pin Number
      buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
      mySerial.write(buffer, 3);

      if ((pinNumber >= 10) && (pinNumber <= 19))
      {
        // board1 pin test
        digitalWrite(board_led[pinNumber], LOW);
      }

      commMode = 1;

      Serial.print("Send Test Pin");
      Serial.println(pinNumber);
    }
    else if (1 == commMode) // recv mode
    {
       if (!PacketCheck('P'))
          return;

        // check pin
        int pin = (int)readBuffer[1];
        if ((pin < 0) || (pin > 19))
          return;

        led_check[ pin] = (readBuffer[2] == 'S')? true : false;

        ++pinNumber;
        commMode = 0;
        
        Serial.print("PinTest ");
        Serial.print(pin);
        Serial.print("=");
        Serial.println(readBuffer[2]);
    }
  }
  else if (5 == state) // Finish pin test
  {
    Serial.println("Finish Pin Test" );
    
    // final check pin
    bool isSuccess = true;
    for (int i=0; i < 20; ++i)
    {
      if (!led_check[i])
        isSuccess = false;      
    }

    if (isSuccess)
    {
      state = 7; // success blink mode

      char buffer[3];
      buffer[0] = 'B';
      buffer[1] = '0';
      buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
      mySerial.write(buffer, 3);
    }
    else
    {
      state = 6; // error mode

       // LED Off
      for (int i=0; i < 10; ++i)
      {
        digitalWrite(board_led[i], HIGH);
      }
      
      char buffer[3];
      buffer[0] = 'I';
      buffer[1] = '0'; // LED Off
      buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
      mySerial.write(buffer, 3);
      delay(100);
          
      // Error LED On
      for (int i=0; i < 20; ++i)
      {
        if (!led_check[i])
        {
          char buffer[3];
          buffer[0] = 'L';
          buffer[1] = i;
          buffer[2] = (char)(((int)buffer[0] + (int)buffer[1]) % 256);
          mySerial.write(buffer, 3);

          if ((i >= 10) && (i <= 19))
          {
            // board1 led on
            pinMode(board_led[i], OUTPUT);
            digitalWrite(board_led[i], LOW);
          }
          
          Serial.print("Error Pin = " );
          Serial.println(i+1);
          delay(100);          
        }
      } // for
      
    } // else

  }
  else if (6 == state)
  {
    
  }
  else if (7 == state)
  {
    blinkType = !blinkType;
    
    for (int i=0; i < 10; ++i)
    {
      digitalWrite(board_led[i], blinkType? HIGH : LOW);
    }

    delay(500);
  }
  
}

