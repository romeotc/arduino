#include <SoftwareSerial.h>
#include <DebouncedSwitch.h>

#define ARDUINO_RX 14  //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 12  //connect to RX of the module

SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX);
//#define mp3 Serial3    // Connect the MP3 Serial Player to the Arduino MEGA Serial3 (14 TX3 -> RX, 15 RX3 -> TX)

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

String mp3Answer;           // Answer from the MP3.

/************ Command byte **************************/
#define CMD_PLAY_W_INDEX  0X03
#define CMD_SEL_DEV       0X09

/************ Opitons **************************/
#define DEV_TF            0X02

void setup()
{
  Serial.begin(9600);
  mp3.begin(9600);
  delay(500);

  sendCommand(CMD_SEL_DEV, DEV_TF);
  delay(500);
}

void loop()
{
  char c = ' ';

  // If there a char on Serial call sendMP3Command to sendCommand
  if ( Serial.available() )
  {
    c = Serial.read();
    sendMP3Command(c);
  }
  sw1.update(); // call this every loop to update switch state
  sw2.update(); // call this every loop to update switch state
  sw3.update(); // call this every loop to update switch state
  
  if (sw.isDown()) { // debounced switch is down
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);      
  }
  
  delay(100);
}

void sendMP3Command(char c) {
  switch (c) {
        
      case '1':
      Serial.println("Play file 1");
      sendCommand(CMD_PLAY_W_INDEX, 0x0001);
      break;
      
      case '2':
      Serial.println("Play file 2");
      sendCommand(CMD_PLAY_W_INDEX, 0x0002);
      break;

      case '3':
      Serial.println("Play file 4");
      sendCommand(CMD_PLAY_W_INDEX, 0x0003);
      break;
      
      case '4':
      Serial.println("Play file 4");
      sendCommand(CMD_PLAY_W_INDEX, 0x0004);
      break;

      case '5':
      Serial.println("Play file 5");
      sendCommand(CMD_PLAY_W_INDEX, 0x0005);
      break;
  }
}

void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e;   //
  Send_buf[1] = 0xff;   //
  Send_buf[2] = 0x06;   // Len
  Send_buf[3] = command;//
  Send_buf[4] = 0x01;   // 0x00 NO, 0x01 feedback
  Send_buf[5] = (int8_t)(dat >> 8);  //datah
  Send_buf[6] = (int8_t)(dat);       //datal
  Send_buf[7] = 0xef;   //
  Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(Send_buf[i]) ;
   
  }
  Serial.println();
}



