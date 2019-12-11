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
#define WIFI_LED 10

// define configuration (number of switches and number of timers)
#define SWITCH_CNT 4
#define TIME_CNT 4

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "Cn0BJasjmsEh9MpRZLplD5cKYIc0Ouu3";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "16/1_2.4Ghz";
char pass[] = "1122334455";


byte switch_pins[] = {14 , 12 , 16 , 13}; // number of gpio to be used as switch/relay control

bool switch_default[] = {LOW,LOW,LOW,LOW}; // switches that use reverse polarity should be set to HIGH here

////////////////////////////////////////////////////////////////////////////////////////
// This code can control up to 4 switches                                            //
// For each switch up to 4 schedule start and end times can be configured - (V0..V15) //
// A default duration can be defined per switch                           - (V16..V19)//
// If an end time is not defined at the app the default duration is used              //
// default duration is also used when manually activating the switch       - (V20..V23)//
// for each switch when start time is reached the switch turns on for the duration    //
//                                                                                    //
// maximum duration is used for safety (to limit activation time)          - (V24..V27)//
////////////////////////////////////////////////////////////////////////////////////////
int  start_time_sec[SWITCH_CNT][TIME_CNT];     // array of 4 start times (in seconds) for 4 switches [switch number][schedual timer number]
bool start_valid[SWITCH_CNT][TIME_CNT];        // is the start time valid ?
bool weekdays[SWITCH_CNT][TIME_CNT][8];        // array of 8 days (day 0 not used) for each schedual time
int  active_duration[SWITCH_CNT][TIME_CNT+1];  // duration per switch per time(in sec)
bool end_valid[SWITCH_CNT][TIME_CNT];          // is the end time valid ?
int  max_duration[SWITCH_CNT];                 // max duration per switch
int auto_off = 1;           // 1 is auto or on

// when activating a switch a timer is set for the configured duration
// when the duration ends the switch is turned off by the timer
// the id of the timer is saved using end_timer_id
// if switch is manually turned off the end_timer_id is used to stop the timer.

// end_timer_id is initialized to 32 (per entry) at the setup section
int end_timer_id[SWITCH_CNT];

// timer object declaration
BlynkTimer timer;

// this code use Real Time Clock widget in the blynk app to keep the clock updated from net
WidgetRTC rtc;


BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncAll();
}
WidgetLED led1(V55);

bool ledStatus = auto_off;



// V55 LED Widget is ON or OFF
void LedWidget()
{ledStatus = auto_off;
  if (ledStatus) {
   led1.on();
  } else {
    led1.off();
    
  }
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



  //////////////////////////////////////////////////////////////////////////////
  // check if end time is received convert and save it as day time in seconds //
  //////////////////////////////////////////////////////////////////////////////
  if (t.hasStopTime())
  {
    
    Serial.println(String("Stop: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
    Serial.println(String("Stop in sec: ") + param[1].asLong() + String(" for switch") + switch_no + String(" time_no: ") + time_no);

    active_duration[switch_no][time_no] = (param[1].asLong()-start_time_sec[switch_no][time_no]);
    
    // if end time is smaller than start time this means end time is at next day
    if (active_duration[switch_no][time_no]<0) active_duration[switch_no][time_no]=86400+active_duration[switch_no][time_no];
    
    Serial.println(String("Stop duration: ") + active_duration[switch_no][time_no]);

    end_valid[switch_no][time_no] = true;


  }
  else // if end time is not defined //
  {
    // Do nothing
    Serial.println(String("No Stop Time Given for switch: ") + switch_no + String(" time_no: ") + time_no);
    end_valid[switch_no][time_no] = false;
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
//this reads status of on/off mode and displays on screen
BLYNK_WRITE(V50) { int Vpin50 = param.asInt();
  auto_off = Vpin50;}


// V0..V3 for switch 0, V4..V7 for switch 1 ...
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

// use a slider to define default activation duration (slider count in minute)
BLYNK_WRITE(V16) { active_duration[0][TIME_CNT] = param.asInt()*60; }
BLYNK_WRITE(V17) { active_duration[1][TIME_CNT] = param.asInt()*60; }
BLYNK_WRITE(V18) { active_duration[2][TIME_CNT] = param.asInt()*60; }
BLYNK_WRITE(V19) { active_duration[3][TIME_CNT] = param.asInt()*60; }


// use a slider to define default max duration (slider count in minute)   ***HARD CODE THIS*** NL
BLYNK_WRITE(V24) { max_duration[0] = param.asInt()*60; }
BLYNK_WRITE(V25) { max_duration[1] = param.asInt()*60; }
BLYNK_WRITE(V26) { max_duration[2] = param.asInt()*60; }
BLYNK_WRITE(V27) { max_duration[3] = param.asInt()*60; }

/////////////////////////////////////////////////////////////////
// Handle switch events (from app or from scheduler )          //
/////////////////////////////////////////////////////////////////

// turn off switch after active duration ends
// duration number is not important here 
void turn_off_switch_no_0(){ turn_on_off(0,0,0); Blynk.virtualWrite(V20,0); Serial.println(String("timer turn off switch 0 ") );}
void turn_off_switch_no_1(){ turn_on_off(0,1,0); Blynk.virtualWrite(V21,0); Serial.println(String("timer turn off switch 1 ") );}
void turn_off_switch_no_2(){ turn_on_off(0,2,0); Blynk.virtualWrite(V22,0); Serial.println(String("timer turn off switch 2 ") );}
void turn_off_switch_no_3(){ turn_on_off(0,3,0); Blynk.virtualWrite(V23,0); Serial.println(String("timer turn off switch 3 ") );}

// handle switch state
void turn_on_off(int on_off, byte switch_no , byte time_no){
    long active_duration_ms ;
    char Time_print[16];
    
    if ((on_off==1) && (auto_off == 1)) //auto_off is a slider in app to shut off the program
    {
      // create time as string to print on activation butten
      sprintf(Time_print, "%02d:%02d", hour(), minute());

      // turn on the switch (or off if default is on)
      digitalWrite(switch_pins[switch_no],!switch_default[switch_no]);

      // if end time is valid use the active duration assigned to this time
      // (change to msec will be done later)
      if (end_valid[switch_no][time_no])
         active_duration_ms = ((long)active_duration[switch_no][time_no]);
      else // otherwise use the general time duration
         active_duration_ms = ((long)active_duration[switch_no][4]);

      
      // max duration smaller than two min is not valid
      if ( (max_duration[switch_no]< 120) | (max_duration[switch_no]>active_duration_ms) )
         active_duration_ms = active_duration_ms*1000;
      else
        active_duration_ms = ((long)max_duration[switch_no])*1000;
       

      // if new timer is set before another one ended then disable previous timer
      if (end_timer_id[switch_no]!=32) timer.deleteTimer(end_timer_id[switch_no]);

      // turn on switch and set timer 
      switch (switch_no) {
        case 0: 
          Blynk.setProperty(V20, "onLabel", String(Time_print));
          Blynk.virtualWrite(V20,1);
          end_timer_id[0]=timer.setTimeout(active_duration_ms, turn_off_switch_no_0);
          
         break;
        case 1: 
          Blynk.setProperty(V21, "onLabel", String(Time_print));
          Blynk.virtualWrite(V21,1);
          end_timer_id[1]=timer.setTimeout(active_duration_ms, turn_off_switch_no_1);
         break;
        case 2: 
          Blynk.setProperty(V22, "onLabel", String(Time_print));
          Blynk.virtualWrite(V22,1);
          end_timer_id[2]=timer.setTimeout(active_duration_ms, turn_off_switch_no_2);
         break;
        case 3: 
          Blynk.setProperty(V23, "onLabel", String(Time_print));
          Blynk.virtualWrite(V23,1);
          end_timer_id[3]=timer.setTimeout(active_duration_ms, turn_off_switch_no_3);
         break;    
      } 
      Serial.println(String("turn ON switch: ") + switch_no + String(" for duration: ") + active_duration_ms/60000 + String("min "));
    }
    else 
    {
      digitalWrite(switch_pins[switch_no],switch_default[switch_no]);
      timer.deleteTimer(end_timer_id[switch_no]);
      end_timer_id[switch_no]=32;
      Serial.println(String("turn OFF switch: ") + switch_no);
    }   
}

// set switch state from APP
BLYNK_WRITE(V20) { turn_on_off(param.asInt(),0,TIME_CNT); }
BLYNK_WRITE(V21) { turn_on_off(param.asInt(),1,TIME_CNT); }
BLYNK_WRITE(V22) { turn_on_off(param.asInt(),2,TIME_CNT); }
BLYNK_WRITE(V23) { turn_on_off(param.asInt(),3,TIME_CNT); }

/////////////////////////////////////////////////////////////////////////////////////////////
// the following function is called every 60 seconds by a timer                            //
//  the function checks if a start time is reached and if yes it will call                 //
//   the turn_on_off function (to turn on the switch and set timer for turning off switch) //
/////////////////////////////////////////////////////////////////////////////////////////////
void activetoday(){         // check if schedule #1 should run today

  if(Blynk.connected()) // set wifi led if no connection
  {
    digitalWrite(WIFI_LED,LOW); 
  }
  else
  {
    digitalWrite(WIFI_LED,HIGH); 
  }
  
  if(year() != 1970){
    unsigned int nowseconds = ((hour() * 3600) + (minute() * 60) + second());
    int dayadjustment = -1;  
    if(weekday() == 1){
      dayadjustment = 6; // needed for Sunday Time library is day 1 and Blynk is day 7
    }
   
 
     for (int switch_cnt = 0;  switch_cnt< SWITCH_CNT; switch_cnt++) {
         for (int timer_cnt = 0;  timer_cnt< TIME_CNT; timer_cnt++) {
              if (start_valid[switch_cnt][timer_cnt] == true) {
                if (weekdays[switch_cnt][timer_cnt][weekday() + dayadjustment]==true){
                  
                   if (nowseconds >= start_time_sec[switch_cnt][timer_cnt]){
                      
                      if(nowseconds < start_time_sec[switch_cnt][timer_cnt]+90){
                        turn_on_off(1,switch_cnt,timer_cnt);
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

// Digital clock display of the time
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

  // Send DATE and DATE to Blynk APP
  Blynk.virtualWrite(V101, currentTime);
  // Send date to the App--Not used but is available--this will show processor date if it is important to you
  Blynk.virtualWrite(V102, currentDate);
}


void setup()
{
  
  Serial.begin(9600);  // Debug console
  
  // reset output pins used and system variables
  for (int i = 0; i<SWITCH_CNT ; i++){
    pinMode(switch_pins[i], OUTPUT); // reset output pins
    digitalWrite(switch_pins[i],switch_default[i]); // set switch to default
    end_timer_id[i] = 32;            // reset end timer id's

    for (int j = 0; j<TIME_CNT ; j++)
    {
      end_valid[i][j] = false;       // reset end valid array
    }
    
  }

  
  pinMode(WIFI_LED, OUTPUT);

  
  Blynk.begin(auth, ssid, pass,"blynk.honey.co.th" ,8080);
    rtc.begin();  // Begin synchronizing time
    timer.setInterval(1000L, clockDisplay);
    timer.setInterval(60000L, activetoday);  // check every 60s if ON / OFF trigger time has been reached
    setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

}

void loop()
{
  Blynk.run();
  timer.run();
  LedWidget();
}
