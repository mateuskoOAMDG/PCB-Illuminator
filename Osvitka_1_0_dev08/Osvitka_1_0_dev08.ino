/**************************************************************************
  Illuminator PCB with count down timer. Arduino project.
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Osvitka s encoderom a OLED 128 x 32
  File   : Osvitka_1_0_dev08.ino
  MCU    : ATMEGA328P
  PCB    : (PINOUT) 1.0.2 rev.12
  Version: 1.0.08
  Date   : 2024-03-31
  Author : mateusko.OAMDG@outlook.com


  Timer for PCB illumination. 
  It uses a 128x32 OLED display and an encoder with a button.
  Setting the time from 1 sec to 99 minutes 59 sec.





 **************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold12pt7b.h>  //vhodný font pre displej
#include <EEPROM.h>

//--------------  configuration ----------------------

#define PIN_LED 9
#define PIN_RELAY A0
#define PIN_BUZZER 10

  /* other used PINs:
    A5 - OLED I2C SCL
    A4 - OLED I2C SDA
    11, 12, 13 - (MOSI, MISO, SCK) - ICSP
  */

  // Display rotation
  //#define DISPLAY_ROTATION 0 //normal
#define DISPLAY_ROTATION 2   // 180 deg


// Encoder Turn Direction - reverse: A 3, B 2
#define ENC_A 2
#define ENC_B 3

// -------------- end configuration -------------------

// do not change!
#define NUM_OF_PRESETS 4
#define GROUP_BUTTONS 1 // tlačidlá predvoľby
#define GROUP_TIMER   2 // timer

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)

//OLED display instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "encoder.h"
#include "Button.h"
#include "Views.h"
#include "TimerV.h"

// Encoder button (switch)
RealButton sw(4);

#define VIEW_TIMER_XOFFSET 30
#define VIEW_BUTTONS_XOFFSET 0

// View instances
TimeView wminutes(VIEW_TIMER_XOFFSET, 24, 29, 19); //minutes
LabelView  wcolon(VIEW_TIMER_XOFFSET + 30, 22, 5, 19); //colon
TimeView wseconds(VIEW_TIMER_XOFFSET + 38, 24, 29, 19); //seconds
TimerV cdTimer;

ButtonView wpreset1(VIEW_BUTTONS_XOFFSET, 0, 15, 15, '1'); //preset button 1
ButtonView wpreset2(VIEW_BUTTONS_XOFFSET + 15, 0, 15, 15, '2'); //preset button 2
ButtonView wpreset3(VIEW_BUTTONS_XOFFSET, 16, 15, 15, '3'); //preset button 3
ButtonView wpreset4(VIEW_BUTTONS_XOFFSET + 15, 16, 15, 15, '4'); //preset button 4

IconView wPlay(97, 2, PLAY_Icon); // Play & Stop Icon
ViewContainer screenMain(&wpreset1, &wPlay, &wPlay); // Main Screen (container)


bool lighting = false; //osvit aktívny

//////////////////////// INVITE SCREEN /////////////////////////////

/// @brief Invite Screen
void inviteScreen() {
  LabelView wInvite(0, 16, 128, 16); 
  
  display.clearDisplay();
  display.setFont(&FONT);
  display.setCursor(0, 16);
  display.setTextColor(1);
  display.print(F(" OSVITKA"));
  display.display();


  display.setFont();
  display.setCursor(0, 23);
  display.print(F("by mateusko 2024 v1.0"));
  display.display();
}


/////////////////////////////////// LED //////////////////////////

/// @brief Led Blink function
/// @param count how many blink
void ledBlink(byte count) {
  int l = digitalRead(PIN_LED);
  for (byte i = 0; i < 2 * count; i++ ) {
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    delay(50);
  }
  digitalWrite(PIN_LED, l);
}

////////////////////////// EEPROM presets ///////////////////////

/// @brief Save preset time to EEPROM
/// @param preset number of preset memory
/// @param timesec saved value  - time in seconds
/// @return succes true|false
bool saveToEEPROM(byte preset, unsigned int timesec) {
  if (preset >= NUM_OF_PRESETS) return false;
  int addr = preset * sizeof(timesec);
  EEPROM.put(addr, timesec);
  return true;
}

/// @brief Load preset time from EEPROM
/// @param preset number of preset memory
/// @return value - time in seconds or 0
unsigned int loadFromEEPROM(byte preset) {
  if (preset >= NUM_OF_PRESETS) return 0;
  unsigned int timesec;
  int addr = preset * sizeof(timesec);
  
  EEPROM.get(addr, timesec);
  if (timesec > 99 * 60 + 59) timesec = 0;
  return timesec;
}

///////////////////////////// HELPERS /////////////////////////

/// @brief Beep function
/// @param time tone time
/// @param pause pause time
void beep(int time = 200, int pause = 100) {
  tone(PIN_BUZZER, 4000, time);
  delay(time);
  noTone(PIN_BUZZER);
  delay(pause);
}

/// @brief Beep function on Stop event 3x3
void beepStop() {
  beep();beep();beep();delay(250);
  beep();beep();beep();delay(250);
  beep();beep();beep();delay(250);
}

/// @brief Short beep on encoder turn
void shortBeep() {
 digitalWrite(PIN_BUZZER, HIGH);
 delay(1);
 digitalWrite(PIN_BUZZER, LOW);
}

/// @brief Beep on Save event 2x
void saveBeep() {
  beep(100, 100); beep(100, 100);
}

/// @brief Start illumination,
void start() {
  if (!cdTimer.isZero()) {
      cdTimer.start();
      wminutes.drawmode &= ~mHIDE;
      wseconds.drawmode &= ~mHIDE;
      digitalWrite(PIN_LED, HIGH);
      digitalWrite(PIN_RELAY, HIGH);
      wPlay.setIcon(STOP_Icon);
      wPlay.draw();
      lighting = true;
      screenMain.forEach(GROUP_BUTTONS | GROUP_TIMER, lockTimerAndPresets);
      display.display();
      beep();beep();
    }
}


/// @brief End illumination
/// @param user true = short beep; false long beep
void stop(bool user = false) {
    //relay and Led off
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_RELAY, LOW);
    lighting = false;
    
    //timer stop
    cdTimer.stop();

    //set Play Icon
    wPlay.setIcon(PLAY_Icon);

    //draw "KONIEC"
    LabelView wEndLighting(0, 24, 128, 19);
    display.clearDisplay();
    wEndLighting.setText(F("  KONIEC"));
    wEndLighting.draw();
    display.display();
    
    if (user) beep(); else beepStop();

    //wait for Click
    unsigned int timer = millis();
    sw.reset();
    while (!sw.onRelease()) sw.update();
    
    display.clearDisplay();
    wminutes.drawmode &= ~mHIDE;
    wseconds.drawmode &= ~mHIDE;
    screenMain.forEach(GROUP_BUTTONS | GROUP_TIMER, unlockTimerAndPresets);
    screenMain.draw();
}

/// @brief Pause Illumination
void pause() {
LabelView wEndLighting(0, 24, 128, 19);
    cdTimer.pause();
    wPlay.setIcon(PLAY_Icon);
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_RELAY, LOW);
    lighting = false;
    wPlay.draw();
    screenMain.forEach(GROUP_TIMER, unlockTimerAndPresets);
    display.display();
    beep();beep();
}


void lockTimerAndPresets(View* view) {
  view->mode |= VIEW_MODE_NOT_SELECTABLE;
}

void unlockTimerAndPresets(View* view) {
  view->mode &= ~VIEW_MODE_NOT_SELECTABLE;
}


/////////////////// View Callback function /////////////////////

/// @brief Callback function for Long press Preset buttons
/// @param preset_no 
/// @param button 
void save(byte preset_no, ButtonView& button) {
  if (preset_no < 1 || preset_no > NUM_OF_PRESETS) return;
  
  if (!saveToEEPROM(preset_no - 1, ((unsigned int) (cdTimer.minutes)) * 60 + cdTimer.seconds )) return;
  
  button.drawmode = mINVERSE | mCLEAR;
  button.draw();
 
  screenMain.setValue(GROUP_BUTTONS, 0);
  button.value = 1;
  screenMain.draw(GROUP_BUTTONS);
   saveBeep();
  ledBlink(5);
}

/// @brief Callback function for Click Preset buttons
/// @param preset_no 
/// @param button 
void load(byte preset_no, ButtonView& button) {
  if (preset_no < 1 || preset_no > NUM_OF_PRESETS) return;
  int sec = loadFromEEPROM(preset_no - 1);
  
  button.drawmode = mINVERSE | mCLEAR;
  button.draw();
  if(sec)  ledBlink(3);
  cdTimer.setTime(sec / 60, sec % 60);
  screenMain.setValue(GROUP_BUTTONS, 0);
  button.value = 1;
  screenMain.draw(GROUP_BUTTONS);
  cdTimer.draw();
}

void wPreset1_onClick() {
  load(1, wpreset1);
}

void wPreset1_onLong() {
  save(1, wpreset1);
}

void wPreset2_onClick() {
  load(2, wpreset2);
}

void wPreset2_onLong() {
  save(2, wpreset2);
}

void wPreset3_onClick() {
  load(3, wpreset3);
}

void wPreset3_onLong() {
  save(3, wpreset3);
}

void wPreset4_onClick() {
  load(4, wpreset4);
}

void wPreset4_onLong() {
  save(4, wpreset4);
}

void wMinutes_onClick() {
  wminutes.drawmode &= ~mHIDE;
  if(wminutes.isEdit())  
    wminutes.setEdit(false);
  else 
    wminutes.setEdit(true);
}

void wSeconds_onClick() {
  wseconds.drawmode &= ~mHIDE;
  if(wseconds.isEdit())  
    wseconds.setEdit(false);
  else 
    wseconds.setEdit(true);
}

void wMinutes_onTurn(int count) {
  cdTimer.addMinutes(count);
  cdTimer.draw();
}

void wSeconds_onTurn(int count) {
  cdTimer.addSeconds(count);
  cdTimer.draw();
}

void wPlay_onClick() {
  if ((cdTimer.mode == TIMERV_MODE_STOP) || (cdTimer.mode == TIMERV_MODE_PAUSE)) {
    start();
  }
  else if (cdTimer.mode == TIMERV_MODE_START) {
    pause();
  }
}

void wPlay_onLong() {
  if(cdTimer.mode != TIMERV_MODE_STOP) stop(true);
}
//////////////////////////// MAIN //////////////////////////////

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Init display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x32
    for (;;) {
      digitalWrite(PIN_LED, HIGH);
      delay(50);
      digitalWrite(PIN_LED, LOW);
      delay(50);
    }
      ;  // Don't proceed, loop forever
  }
   
  display.setRotation(DISPLAY_ROTATION);

  inviteScreen();
  delay(1000);
  
  display.invertDisplay(false);
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.cp437(true);

  // Init Encoder
  encoder_init();

  // Init View Instances
  wminutes.nextView = &wcolon;
  wminutes.prevView = &wpreset4;
  wminutes.onClick = wMinutes_onClick;
  wminutes.onTurn  = wMinutes_onTurn;
  wminutes.group   = GROUP_TIMER;

  wcolon.nextView = &wseconds;
  wcolon.prevView = &wminutes;
  wcolon.setText(F(":"));
  wcolon.xTextOffset = -3;

  wseconds.nextView = &wPlay;
  wseconds.prevView = &wcolon;
  wseconds.onClick = wSeconds_onClick;
  wseconds.onTurn  = wSeconds_onTurn;
  wseconds.group   = GROUP_TIMER;


  wpreset1.nextView = &wpreset2;
  wpreset1.prevView = &wPlay;
  wpreset1.onClick = wPreset1_onClick;
  wpreset1.onLong  = wPreset1_onLong;
  wpreset1.group = GROUP_BUTTONS;

  wpreset2.nextView = &wpreset3;
  wpreset2.prevView = &wpreset1;
  wpreset2.onClick = wPreset2_onClick;
  wpreset2.onLong  = wPreset2_onLong;
  wpreset2.group = GROUP_BUTTONS;

  wpreset3.nextView = &wpreset4;
  wpreset3.prevView = &wpreset2;
  wpreset3.onClick = wPreset3_onClick;
  wpreset3.onLong  = wPreset3_onLong;
  wpreset3.group = GROUP_BUTTONS;

  wpreset4.nextView = &wminutes;
  wpreset4.prevView = &wpreset3;
  wpreset4.onClick = wPreset4_onClick;
  wpreset4.onLong  = wPreset4_onLong;
  wpreset4.group = GROUP_BUTTONS;

  wPlay.nextView = &wpreset1;
  wPlay.prevView = &wseconds;
  wPlay.onClick = wPlay_onClick;
  wPlay.onLong = wPlay_onLong;

  cdTimer.setView(&wminutes, &wseconds, &wcolon);
  load(1, wpreset1);
  screenMain.draw();
  cdTimer.draw();
  display.display();  
}

void loop() {
  int i = encoder_impuls(); //get event from encoder
  if (i < 1000 && i !=0) shortBeep(); // short beep on encoder turn
  if (i) {
    screenMain.doEvent(i); //do event
    display.display();
  }

  else if (sw.onClick()) { //Click encoder button event
    screenMain.doEvent(VIEW_EVENT_CLICK);
    display.display();
  }

  else if (sw.onLong()) { //Long click encoder button event
    screenMain.doEvent(VIEW_EVENT_LONG);
    display.display();
  }

  if (cdTimer.isZero() && lighting) {
    stop();
  }
  
  sw.update();
  cdTimer.update();
}
