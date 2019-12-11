 /*************************************************************
  This example runs directly on ESP8266 chip.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#define WIFI_LED 13

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "8_-kO5UB-_khzSv76Yk1QaticD0rd_ZA";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "16/1_2.4Ghz";
char pass[] = "1122334455";

byte switch_pins[] = {12 , 5 , 4 , 15}; // number of gpio to be used as switch/relay control

bool isFirstConnect = true;
/////////////////////////////////////////////////////////////////////
// this code can control up to 4 switches                          //
// for each switch up to 4 schedule start times can be configured  //
// for each switch one duration is used (when schedule time is     //
//    reached the switch turns on for the duration)                //
/////////////////////////////////////////////////////////////////////
int start_time_sec[4][4];   // array of 4 start times (in seconds) for 4 switches [switch number][schedule timer number]
bool start_valid[4][4];     // is the start time valid ?
bool weekdays[4][4][8];     // array of 8 days (day 0 not used) for each schedule time
byte active_duration[4];    // duration per switch (in minuets)
bool wifi_led_status = false;

// when activating a switch a timer is set for the configured duration
// when the duration ends the switch is turned off by the timer
// the number of the timer is saved using end_timer_id
// if switch is manualy turned off the end_timer_id is used to stop the timer.
int end_timer_id[4] = {32, 32, 32, 32};

// timer object declaration
BlynkTimer timer;

// this code use Real Time Clock widget in the blynk app to keep the clock updated from net
WidgetRTC rtc;


BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncAll();
}

//////////////////////////////////////////////////////////
// get schedual parameters from App                     //
//////////////////////////////////////////////////////////
void set_time(BlynkParam param, byte switch_no, byte time_no){

     TimeInputParam t(param);
  // Process start time

  if (t.hasStartTime())
  {
    
    Serial.println(String("Start: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
    Serial.println(String("Start in sec: ") + param[0].asLong() + String(" for switch") + switch_no + String(" time_no: ") + time_no);
    start_time_sec[switch_no][time_no]= param[0].asLong();
    start_valid[switch_no][time_no] = true;


  }
  else
  {
    // Do nothing
    Serial.println(String("No Start Time Given for switch: ") + switch_no + String(" time_no: ") + time_no);
    start_valid[switch_no][time_no] = false;
  }

 
  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)

  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      Serial.println(String("Day ") + i + " is selected");
      weekdays[switch_no][time_no][i] = true;
    }
    else {
      weekdays[switch_no][time_no][i] = false;
    }
    
  }

    Serial.println();

}


BLYNK_WRITE(V0)  { set_time(param, 0,0);  }
BLYNK_WRITE(V1)  { set_time(param, 0,1);  }
BLYNK_WRITE(V2)  { set_time(param, 0,2);  }
BLYNK_WRITE(V3)  { set_time(param, 0,3);  }

BLYNK_WRITE(V4)  { set_time(param, 1,0);  }
BLYNK_WRITE(V5)  { set_time(param, 1,1);  }
BLYNK_WRITE(V6)  { set_time(param, 1,2);  }
BLYNK_WRITE(V7)  { set_time(param, 1,3);  }

BLYNK_WRITE(V8)  { set_time(param, 2,0);  }
BLYNK_WRITE(V9)  { set_time(param, 2,1);  } 
BLYNK_WRITE(V10) { set_time(param, 2,2);  }
BLYNK_WRITE(V11) { set_time(param, 2,3);  }

BLYNK_WRITE(V12) { set_time(param, 3,0);  }
BLYNK_WRITE(V13) { set_time(param, 3,1);  }
BLYNK_WRITE(V14) { set_time(param, 3,2);  }
BLYNK_WRITE(V15) { set_time(param, 3,3);  }





BLYNK_WRITE(V16) { active_duration[0] = param.asInt(); }
BLYNK_WRITE(V17) { active_duration[1] = param.asInt(); }
BLYNK_WRITE(V18) { active_duration[2] = param.asInt(); }
BLYNK_WRITE(V19) { active_duration[3] = param.asInt(); }

/////////////////////////////////////////////////////////////////
// Handle switch events (from app or from scheduler )          //
/////////////////////////////////////////////////////////////////

// turn off switch after active duration ends
void turn_off_switch_no_0(){ turn_on_off(0,0); Blynk.virtualWrite(V20,0); Serial.println(String("timer turn off switch 0 ") );}
void turn_off_switch_no_1(){ turn_on_off(0,1); Blynk.virtualWrite(V21,0); Serial.println(String("timer turn off switch 1 ") );}
void turn_off_switch_no_2(){ turn_on_off(0,2); Blynk.virtualWrite(V22,0); Serial.println(String("timer turn off switch 2 ") );}
void turn_off_switch_no_3(){ turn_on_off(0,3); Blynk.virtualWrite(V23,0); Serial.println(String("timer turn off switch 3 ") );}

// handle switch state
void turn_on_off(int on_off, byte switch_no){
    if (on_off==1) 
    {
      
      digitalWrite(switch_pins[switch_no],HIGH);
      long active_duration_ms = ((long)active_duration[switch_no])*60000;
      
      switch (switch_no) {
        case 0: 
          Blynk.virtualWrite(V20,1);
          end_timer_id[0]=timer.setTimeout(active_duration_ms, turn_off_switch_no_0);
          
         break;
        case 1: 
          Blynk.virtualWrite(V21,1);
          end_timer_id[1]=timer.setTimeout(active_duration_ms, turn_off_switch_no_1);
         break;
        case 2: 
          Blynk.virtualWrite(V22,1);
          end_timer_id[2]=timer.setTimeout(active_duration_ms, turn_off_switch_no_2);
         break;
        case 3: 
          Blynk.virtualWrite(V23,1);
          end_timer_id[3]=timer.setTimeout(active_duration_ms, turn_off_switch_no_3);
         break;    
      } 
      Serial.println(String("turn ON switch: ") + switch_no);
    }
    else 
    {
      digitalWrite(switch_pins[switch_no],LOW);
      timer.deleteTimer(end_timer_id[switch_no]);
      end_timer_id[switch_no]=32;
      Serial.println(String("turn OFF switch: ") + switch_no);
      

    }   
}

// set switch state from APP
BLYNK_WRITE(V20) { turn_on_off(param.asInt(),0); }
BLYNK_WRITE(V21) { turn_on_off(param.asInt(),1); }
BLYNK_WRITE(V22) { turn_on_off(param.asInt(),2); }
BLYNK_WRITE(V23) { turn_on_off(param.asInt(),3); }



// schedual events handling
void activetoday(){         // check if schedule #1 should run today

  if(Blynk.connected())
  {
    digitalWrite(WIFI_LED,LOW); 
  }
  else
  {
    wifi_led_status =!wifi_led_status;
    digitalWrite(WIFI_LED,wifi_led_status); 
  }
  
  if(year() != 1970){
    unsigned int nowseconds = ((hour() * 3600) + (minute() * 60) + second());
    int dayadjustment = -1;  
    if(weekday() == 1){
      dayadjustment = 6; // needed for Sunday Time library is day 1 and Blynk is day 7
    }
   
 
     for (int switch_cnt = 0;  switch_cnt<= 3; switch_cnt++) {
         for (int timer_cnt = 0;  timer_cnt<= 3; timer_cnt++) {
              if (start_valid[switch_cnt][timer_cnt] == true) {
                if (weekdays[switch_cnt][timer_cnt][weekday() + dayadjustment]==true){
                  
                   if (nowseconds >= start_time_sec[switch_cnt][timer_cnt]){
                      
                      if(nowseconds < start_time_sec[switch_cnt][timer_cnt]+90){
                        turn_on_off(1,switch_cnt);
                        Serial.println(String("turn ON switch: ") + switch_cnt);

                      }
                   }
                }
              }
         }
      
     }
 
  } 
}

/////////////////////////////////////
// BLYNK 
void setup()
{
  // Debug console
  Serial.begin(9600);
  for (int i = 0; i<4 ; i++){
    pinMode(switch_pins[i], OUTPUT);
    
  }
  pinMode(WIFI_LED, OUTPUT);


  
  Blynk.begin(auth, ssid, pass, "blynk.honey.co.th",8080);
  timer.setInterval(60000L, activetoday);  // check every 60s if ON / OFF trigger time has been reached
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

}


void loop()
{
  Blynk.run();
  timer.run();
}
