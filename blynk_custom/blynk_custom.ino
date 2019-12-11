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


#include "DHT.h"

#define DHTPIN 2 
BlynkTimer timer;
WidgetRTC rtc;

#define DHTTYPE DHT11   // DHT 11
#define p1 V14
#define p2 V12
float h,t;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "4ZXyvd4_FJ_MLgFhvVhfRcBZEWZO28NP";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "16/1_2.4Ghz";
char pass[] = "1122334455";
int level_set = 0;
DHT dht(DHTPIN, DHTTYPE);


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

void setup()
{
  // Debug console
  Serial.begin(9600);

  //Blynk.begin(auth, ssid, pass);
  Blynk.begin(auth, ssid, pass, "blynk.honey.co.th", 8080);  
  dht.begin();
  // You can also specify server:

  timer.setInterval(2500, Sensor);
  timer.setInterval(2500, Check_level);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  timer.setInterval(10000L, clockDisplay);
    
}

void loop()
{
  Blynk.run();
  timer.run(); // run BlynkTimer
}
void Sensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    
    delay(1500);
    return;
  }
  Serial.print("Humidity is: "); Serial.println(h, 1);
  Serial.print("Temperature is: "); Serial.println(t, 1);
  
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, t);
}
void work()
{
  
  
}

