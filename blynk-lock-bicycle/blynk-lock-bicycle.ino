/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>



BlynkTimer timer;
WidgetRTC rtc;

#define DHTTYPE DHT11   // DHT 11
#define p1 V14
#define p2 V12
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "ymL-5HYhp5BQs_82G_ZHPU3LWGv3wztQ";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "16/1_2.4Ghz";
char pass[] = "1122334455";
String level_set = "";
int button = 0;
String key = "";
int button_set = 0;



void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();

  // Send time to the App
  Blynk.virtualWrite(V1, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V2, currentDate);
}
void show_data()
{
   Serial.print("key=");
  Serial.println(key);
   Serial.print("buttton=");
  Serial.println(button);
  
  Serial.print("level_set=");
  Serial.println(level_set);
      Blynk.virtualWrite(V4, key);
      Blynk.virtualWrite(V3, level_set);
      Blynk.virtualWrite(V0, button);
  
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  Blynk.syncAll();
  rtc.begin();
}
BLYNK_WRITE(V3)
{
  level_set = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println(level_set);

  

  // process received value
}
BLYNK_WRITE(V0)
{
  button = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println(button);
 

  // process received value
}
BLYNK_WRITE(V4)
{
  key = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println(key);

 
  // process received value
}
BLYNK_WRITE(V6)
{
  button_set = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println(button_set);

 
  // process received value
}
///////////////////////

BLYNK_READ(V3)
{
  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
  Blynk.virtualWrite(V0, level_set);
}
BLYNK_READ(V4)
{
  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
  Blynk.virtualWrite(V4, key);
}
BLYNK_READ(V0)
{
  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
  Blynk.virtualWrite(V0, button);
}


void setup()
{
  // Debug console
  Serial.begin(115200);

  //Blynk.begin(auth, ssid, pass);
  Blynk.begin(auth, ssid, pass, "blynk.honey.co.th", 8080);  
  
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
 
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(2500, show_data);
  pinMode(p1,OUTPUT);
  pinMode(p2,OUTPUT);

  
}

void loop()
{
  Blynk.run();
  timer.run(); // run BlynkTimer
  if(button_set==1){
     if(button==0 && level_set!=""  )
    {
      key = level_set;
      Blynk.virtualWrite(V4, level_set);
      Blynk.virtualWrite(V3, "");
      Blynk.virtualWrite(V0, 1);
      button = 1;
      level_set="";
      delay(1000);
    }
    else if(button==1 && level_set==key )
    {
      key = "";
      level_set = "";
      Blynk.virtualWrite(V4, "");
      Blynk.virtualWrite(V3, "");
      Blynk.virtualWrite(V0, 0);
      button = 0;
      delay(1000);
    }
  }
  
 
}



