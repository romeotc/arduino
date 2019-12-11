#include <SoftwareSerial.h>
//#include <DebouncedSwitch.h>

#define ARDUINO_RX 14  //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 12  //connect to RX of the module


// constants won't change. They're used here to set pin numbers:
const int button1 = 5;    // the number of the pushbutton pin
const int button2 = 4; 
const int button3 = 0; 

char c = ' ';
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

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);


  
}

void loop()
{
  //char c = ' ';
   
  if (digitalRead(button1)==LOW) { // debounced switch is down
   c = '1';
  }
  if (digitalRead(button2)==LOW) { // debounced switch is down
   c = '2';
  }
  if (digitalRead(button3)==LOW) { // debounced switch is down
   c = '3';
  }
  
  

  // If there a char on Serial call sendMP3Command to sendCommand
  if ( c!=' ' )
  {
     sendMP3Command(c);
     c = ' ';
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
      

  }
  c = ' ';
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
    Serial.print(Send_buf[i]);
  }
  Serial.println();
}



