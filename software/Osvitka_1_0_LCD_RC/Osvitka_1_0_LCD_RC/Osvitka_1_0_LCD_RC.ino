/**************************************************************************
  Illuminator PCB with Countdown Timer. 
  LCD 16x2 & Encoder with Switch. 
  Arduino project.
  
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Illuminator PCB with Encoder & LCD 16 x 2
  File   : Osvitka_1_0_LCD_dev09.ino
  MCU    : ATMEGA328P
  PCB    : (PINOUT) 1.1.0 rev. 01
  Version: 1.1.0 dev 01
  Date   : 2024-05-03
  Author : mateusko.OAMDG@outlook.com


  Timer for PCB illumination. 
  It uses a LCD 16 x 2 display and an encoder with a button.
  Time setting from 1 sec to 99 minutes & 59 sec.



 **************************************************************************/

#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#include "RealButton.h" //handles encoder switch pressing

#include "Encoder.h"    // encoder rotation handler
#include "Views.h"      // classes for handling display rendering
#include "TimerV.h"     // class for countdown timer

//--------------  configuration ----------------------

#define PIN_LED     9   // LED
#define PIN_RELAY   A0  // Relay
#define PIN_BUZZER  10  // Passive Buzzer
#define PIN_SW      4   // Encoder Switch

/* Encoder rotation direction. Uncomment one option */

#define ENCODER_ROTATION_DIRECTION 1
// #define ENCODER_ROTATION_DIRECTION -1


/* Beep on rotation (comment out the line if you don't need the beep) */
#define BEEP_WHEN_TURNING 


  /* other used PINs:
    A5 - OLED I2C SCL
    A4 - OLED I2C SDA
    11, 12, 13 - (MOSI, MISO, SCK) - ICSP
    2 - Encoder A
    3 - Encoder B
    
  */

//-------------- end of configuration -----------------






Encoder encoder;          // Rotation Encoder instance
RealButton sw(PIN_SW);    // Encoder Switch Button instance
LiquidCrystal_I2C lcd(0x27, 16, 2); //  LCD instance

bool light = false;       // the state of the exposure unit Relay

int last_start_time = 0;  // memory for last start time (sec)

//Display rendering instances
  TimerV timer;
  TimeView wmin(1, 0);
  TimeView wsec(4, 0);
  LabelView wcolon(3, 0, 1, ":");
  ButtonView btnP1(1, 1, 2, "P1");
  ButtonView btnP2(5, 1, 2, "P2");
  ButtonView btnP3(9, 1, 2, "P3");
  ButtonView btnP4(13, 1, 2, "P4");
  ButtonView btnStart(10, 0, 5, "Start");
  ButtonView btnStop(10, 0, 4, "Stop");
  ViewContainer screen1;

//////////// Beep functions ///////////

void beep(int time = 200, int pause = 100, bool led = false, int f = 4000) {
  if (led) digitalWrite(PIN_LED, HIGH);
  tone(PIN_BUZZER, f, time);
  delay(time);
  if (led) digitalWrite(PIN_LED, LOW);
  noTone(PIN_BUZZER);
  delay(pause);
}

void beepInit() {
  beep(100,100,true,3400);
  beep(100,100,true,4410);
  beep(100,100,true,5244);

}

void beepSave() {
  beep(200, 100, true); 
  beep(100, 100, true);
}

void beepLoad() {
  beep(100, 100, true);
}

void beepStopZero() {
  beep(200, 100, true); 
  beep(200, 100, true); 
  beep(200, 300, true); 
  beep(200, 100, true); 
  beep(200, 100, true); 
  beep(200, 300, true); 
  beep(200, 100, true); 
  beep(200, 100, true); 
  beep(200, 300, true); 

}
void beepStart() {
  beep(200, 100, true); 
  beep(200, 100, true); 
}

void beepStopUser() {
  beep(200, 100, true); 
  beep(200, 100, true); 
  beep(200, 100, true); 
}

////////////// EEPROM functions //////////////

void loadFromEEPROM(byte preset) { //preset: 1,2,3,4
  preset --;
  if (preset >= 4) return;
  
  unsigned int timesec;
  int addr = preset * sizeof(timesec);
  
  EEPROM.get(addr, timesec);
  if (timesec > 99 * 60 + 59) timesec = 0;
  timer.setTime(timesec / 60, timesec % 60);
  beepLoad();
}

void saveToEEPROM(int preset) {
  preset --;
  if (preset >= 4) return;

  unsigned int timesec;
  timesec = timer.minutes * 60 + timer.seconds;

  int addr = preset * sizeof(timesec);
  EEPROM.put(addr, timesec);
  beepSave();
}

//////////// Display Rendering Helpers ///////////

void activatePx(bool active = true) {
  btnP1.setVisible(active);
  btnP2.setVisible(active);
  btnP3.setVisible(active);
  btnP4.setVisible(active);
}

void activateTimer(bool active = true) {
  wmin.setSelectable(active);
  wsec.setSelectable(active);
}

//////////// Display Rendering Callback Functions //////////

  // Start Button Click
void start_callback() {
  if (timer.isZero()) return;
  activatePx(false);
  activateTimer(false);
  beepStart();
  last_start_time = timer.minutes * 60 + timer.seconds;
  
  btnStart.setVisible(false);
  btnStop.setVisible(true);
  screen1.setSelected(btnStop);
  screen1.draw(0, true);
  timer.start();
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_RELAY, HIGH);
  light = true;
  lcd.setCursor(1,1);
  lcd.print(F("Prebieha osvit"));
}

  // Minutes Editing
void minute_edit_callback(int event) {
  if (!event) return;
  if (event <= 1000 && event >= -1000) {
    timer.addMinutes(event);
    wmin.draw();
  }

}

  // Seconds Editing
void seconds_edit_callback(int event) {
  if (!event) return;
  if (event <= 1000 && event >= -1000) {
    timer.addSeconds(event);
    wmin.draw();
    wsec.draw();
  }
}

  // Stop Button Click
void stop_callback() {
  if (!light) return;
  light = false;
  
  timer.stop(); 
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_RELAY, LOW);
  lcd.setCursor(0,1);
  lcd.print(F("     KONIEC     "));
  if (timer.isZero()) {
    timer.setTime(last_start_time / 60, last_start_time % 60);
    beepStopZero();
    delay(500);
  } 
  else {
    beepStopUser();
    delay(500);
  } 
  activatePx(true);
  activateTimer(true);
  digitalWrite(PIN_LED, LOW);
  
  btnStop.setVisible(false);
  btnStart.setVisible(true);
  screen1.setSelected(btnStart);
  screen1.draw(0, true);
 
  wcolon.setClear(false);
  timer.draw();
}

  // Preset 1
void p1_click_callback() {
  loadFromEEPROM(1);
}

void p1_long_callback() {
  saveToEEPROM(1);
}

  // Preset 2
void p2_click_callback() {
  loadFromEEPROM(2);
}

void p2_long_callback() {
  saveToEEPROM(2);
}

  // Preset 3
void p3_click_callback() {
  loadFromEEPROM(3);
}

void p3_long_callback() {
  saveToEEPROM(3);
}

  // Preset 4
void p4_click_callback() {
  loadFromEEPROM(4);
}

void p4_long_callback() {
  saveToEEPROM(4);
}

///////////// Display Rendering Initialisation ///////////////

void initInstances() {
  btnStart.onClick = start_callback;
  btnStop.onClick  = stop_callback;
  
  screen1.add(wmin);
  screen1.add(wcolon);
  screen1.add(wsec);
  screen1.add(btnStop);
  screen1.add(btnStart);
  screen1.add(btnP1);
  screen1.add(btnP2);
  screen1.add(btnP3);
  screen1.add(btnP4);

  screen1.setSelected(btnStart);
  
  btnStop.setVisible(false);

  btnP1.value   = 1;
  btnP1.onClick = p1_click_callback;
  btnP1.onLong  = p1_long_callback;

  btnP2.value   = 2;
  btnP2.onClick = p2_click_callback;
  btnP2.onLong  = p2_long_callback;

  btnP3.value   = 3;
  btnP3.onClick = p3_click_callback;
  btnP3.onLong  = p3_long_callback;

  btnP4.value   = 4;
  btnP4.onClick = p4_click_callback;
  btnP4.onLong  = p4_long_callback;
 
  timer.setView(&wmin, &wsec, & wcolon);
  wmin.onTurn = minute_edit_callback;
  wsec.onTurn = seconds_edit_callback;
}  

/////////// Invite Screen ////////////

void inviteScreen() {
  lcd.clear();
  lcd.print(F("<< Osvitka  1 >>"));
  lcd.setCursor(0,1);
  lcd.print(F(" mateusko OAMDG "));
  beepInit();
}

///////// Event collector - gets events from the Encoder and its buttons /////

int getEvent() {
  // getting an event code according to the rotation of the encoder (may by -1 or 1)
  int i = encoder.impuls();
  
  if (i) {
  #ifdef BEEP_WHEN_TURNING
    digitalWrite(PIN_BUZZER, HIGH);
    delay(5);
    digitalWrite(PIN_BUZZER, LOW);
 #endif
    return i;
  }
  
  // creating the event code according to the status of the encoder button
  if (sw.onClick()) return VIEW_EVENT_CLICK;
  if (sw.onLong())  return VIEW_EVENT_LONG;
  if (sw.onRelease())  return VIEW_EVENT_RELEASE;
  return 0;
}

/////////////// SETUP ////////////////

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  lcd.begin();

  inviteScreen();
  delay(2000);
  lcd.clear();
  initInstances();
  loadFromEEPROM(1);
  screen1.draw();
  encoder.setDirection(ENCODER_ROTATION_DIRECTION);
}




void loop() {
  // Event retrieval and processing  
  if (int e = getEvent()) screen1.doEvent(e);

  // End of exposure test
  if (timer.isZero() && light) stop_callback();
  
  // Update
  sw.update();
  timer.update();
}

