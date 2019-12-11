
#define BLYNK_PRINT Serial
#include <DebouncedSwitch.h>

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <SoftwareSerial.h>

#define ARDUINO_RX 14  //should connect to TX of the Serial MP3 Player module D5
#define ARDUINO_TX 12  //connect to RX of the module D6

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

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "KMh5u-U53QLioUNz0-P7GiJy2puh4M6i";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ict";
char pass[] = "1122334455";
char c = ' ';

  DebouncedSwitch sw1(5); // monitor a switch on input D1
  DebouncedSwitch sw2(4); // monitor a switch on input D2
  DebouncedSwitch sw3(0); // monitor a switch on input D3

void setup()
{
  // Debug console
  Serial.begin(9600);
 
  mp3.begin(9600);
  delay(500);

  sendCommand(CMD_SEL_DEV, DEV_TF);
  delay(500);

  Blynk.begin(auth, ssid, pass, "blynk.honey.co.th",8080);

}

BLYNK_WRITE(V1) { c = '1'; sendMP3Command(c); c = ' '; }
BLYNK_WRITE(V2) { c = '2'; sendMP3Command(c); c = ' '; }
BLYNK_WRITE(V3) { c = '3'; sendMP3Command(c); c = ' '; }

void loop()
{
  sw1.update(); // call this every loop to update switch state
  sw2.update(); // call this every loop to update switch state
  sw3.update(); // call this every loop to update switch state
  Blynk.run();
  
   if ( Serial.available() )
    {
      c = Serial.read();
      sendMP3Command(c);
    }
    
  if (sw1.isDown()) { // debounced switch is down
    c = '1';
    sendMP3Command(c);
  } 
  if (sw2.isDown()) { // debounced switch is down
    c = '2';
    sendMP3Command(c);
  } 
  if (sw3.isDown()) { // debounced switch is down
    c = '3';
    sendMP3Command(c);
  } 
}

void sendMP3Command(char d) {
  switch (d) {
        
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




