/**************************************************************************
  Illuminator PCB with Countdown Timer. 
  LCD 16x2 & Encoder with Switch. 
  Arduino project.
  
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Illuminator PCB with Encoder & LCD 16 x 2
  File   : TimerV.h
  MCU    : ATMEGA328P
   
  
  Version: tmrv 1.01 LCD
  Date   : 2024-05-03
  Author : mateusko.OAMDG@outlook.com

  Count-down Timer Class definition

**************************************************************************/
#include <Arduino.h>
#include "TimerV.h"
#include <LiquidCrystal_I2C.h>

/// @brief global instance of display
extern LiquidCrystal_I2C lcd;

/// @brief Constructor
TimerV::TimerV(){
  minutes = 0;
  seconds = 0;
  mode = TIMERV_MODE_STOP;
  minutes_view = NULL;
  seconds_view = NULL;
  colon_view   = NULL;
}

/// @brief Set View instances for min, sec and collon displaying 
/// @param wminutes 
/// @param wseconds 
/// @param wcolon 
void TimerV::setView(TimeView* wminutes, TimeView* wseconds, LabelView* wcolon) {
  minutes_view = wminutes;
  seconds_view = wseconds;
  colon_view   = wcolon;
}

/// @brief Start Count Down Timer
void TimerV::start() {
  mode = TIMERV_MODE_START;
  timer = millis();
}

/// @brief Stop Count Down Timer
void TimerV::stop() {
  mode = TIMERV_MODE_STOP;
  draw();
  

}

/// @brief Pause Count Down Timer
void TimerV::pause() {
  timer = millis();
  mode = TIMERV_MODE_PAUSE;
  draw();
}

/// @brief Add minutes
/// @param count 
/// @return overflow true|false
bool TimerV::addMinutes(int count) {
  bool over = false;
  minutes += count;
  if (minutes > 99) {
    minutes = 99;
    over = true;
  }
  else if (minutes < 0) {
    minutes = 0;
    over = true;
  }
  minutes_view->time = minutes;
  seconds_view->time = seconds;
  return over; //true is overflow
}


/// @brief Add seconds
/// @param count 
/// @return  overflow true|false
bool TimerV::addSeconds(int count) {
  bool over = false;
  
  seconds += count;
  if (seconds < 0) {
    seconds = -seconds;
    over = addMinutes( - (seconds / 60) - 1);
    if (over) 
      seconds = 0;
    else {
      seconds %= 60;
      seconds = 60 - seconds;
      if (seconds == 60) seconds = 0;
    }
  } 
  else if (seconds >= 60) {
    over = addMinutes(seconds / 60);
    if (over) seconds = 59; else seconds %= 60;

  }
  minutes_view->time = minutes;
  seconds_view->time = seconds;
  return over;
}


/// @brief Set time min, sec
/// @param xminutes 
/// @param xseconds 
/// @return true
bool TimerV::setTime(byte xminutes, byte xseconds) {
  if (xminutes > 99 || xseconds > 59) return false;
  minutes = xminutes;
  seconds = xseconds;
  minutes_view->time = minutes;
  seconds_view->time = seconds;
  return true;
}

/// @brief Update timer. Must by called in loop regularly
void TimerV::update() {
  static bool colon_visible = true;
  unsigned int time = millis();
  if (mode == TIMERV_MODE_START) {
    if (time - timer >= TIMERV_BLINK_COLON_TIME) {
      if (colon_visible) {
        colon_visible = false;
        colon_view -> setClear(true);
        colon_view -> draw();
      } 
    }

    if (time - timer >= TIMERV_SECOND_TIME) {
      timer += TIMERV_SECOND_TIME;
      colon_visible = true;
      colon_view -> setClear(false);
      addSeconds(-1);

      if (isZero()) mode = TIMERV_MODE_STOP;
      draw();
    }
  }
  else if (mode == TIMERV_MODE_PAUSE) {
    if (time - timer >= TIMERV_SPEED_BLINK_COLON) {
      timer = time;
      if (colon_visible) {
        colon_view -> setClear(true);
        if (! (minutes_view -> mode & VIEW_MODE_EDITED)) minutes_view -> setClear(true);
        if (! (seconds_view -> mode & VIEW_MODE_EDITED)) seconds_view -> setClear(true);
        colon_visible = false;
        
      } 
      else {
        colon_view -> setClear(false);
         if (! (minutes_view -> mode & VIEW_MODE_EDITED)) minutes_view -> setClear(false);
        if (! (seconds_view -> mode & VIEW_MODE_EDITED)) seconds_view -> setClear(false);
        colon_visible = true;
      }

      
      draw();
      
    }
  }
}

/// @brief Draws min, sec and collon on display
/// @return true
bool TimerV::draw() {
  if (minutes_view != NULL) {
    
    minutes_view->draw();
  }
  if (seconds_view != NULL) {
    
    seconds_view->draw();
  }
  if (colon_view != NULL)   colon_view->draw();
  return true;
}

// O.A.M.D.G