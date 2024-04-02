/**************************************************************************
  Project: Osvitka s encoderom a OLED 128 x 32
  File   : TimerV.cpp
  MCU    : ATMEGA328P
  
  
  Version: tmrv 0.02
  Date   : 2024-03-31
  Author : mateusko.OAMDG@outlook.com

  Implementaton of classes for count down Timer. Header file: TimerV.h

 **************************************************************************/
#include <Arduino.h>
#include "TimerV.h"
#include <Adafruit_SSD1306.h>

/// @brief global instance of display
extern Adafruit_SSD1306 display;

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
  wcolon -> setText(F(":"));
  wcolon -> mode |= VIEW_MODE_NOT_SELECTABLE;
}

/// @brief Start Count Down Timer
void TimerV::start() {
  mode = TIMERV_MODE_START;
  timer = millis();
}

/// @brief Stop Count Down Timer
void TimerV::stop() {
  mode = TIMERV_MODE_STOP;
  colon_view->drawmode &= ~mHIDE;
  draw();
  display.display();

}

/// @brief Pause Count Down Timer
void TimerV::pause() {
  timer = millis();
  mode = TIMERV_MODE_PAUSE;
  colon_view->drawmode &= ~mHIDE;
  draw();
  display.display();

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
  return true;
}

/// @brief Update timer. Must by called in loop regularly
void TimerV::update() {
  static bool colon_visible = true;
  unsigned int time = millis();
  if (mode == TIMERV_MODE_START) {
    if (time - timer >= TIMERV_BLINK_COLON_TIME) {
      if (colon_visible) {
        colon_view -> drawmode |= mHIDE;
        colon_visible = false;
        colon_view -> draw();
        display.display();
      } 
    }

    if (time - timer >= TIMERV_SECOND_TIME) {
      timer += TIMERV_SECOND_TIME;
      colon_view -> drawmode &= ~mHIDE;
      colon_visible = true;
      addSeconds(-1);

      if (isZero()) mode = TIMERV_MODE_STOP;
      draw();
      display.display();
    }
  }
  else if (mode == TIMERV_MODE_PAUSE) {
    if (time - timer >= TIMERV_SPEED_BLINK_COLON) {
      timer = time;
      if (colon_visible) {
        colon_view -> drawmode |= mHIDE;
        if (! (minutes_view -> mode & VIEW_MODE_EDITED)) minutes_view -> drawmode |= mHIDE;
        if (! (seconds_view -> mode & VIEW_MODE_EDITED)) seconds_view -> drawmode |= mHIDE;
        colon_visible = false;
        
      } 
      else {
        colon_view -> drawmode &= ~mHIDE;
         if (! (minutes_view -> mode & VIEW_MODE_EDITED)) minutes_view -> drawmode  &= ~mHIDE;
        if (! (seconds_view -> mode & VIEW_MODE_EDITED)) seconds_view -> drawmode  &= ~mHIDE;
        colon_visible = true;
      }

      //colon_view -> draw();
      draw();
      display.display();
    }
  }
}

/// @brief Draws min, sec and collon on display
/// @return true
bool TimerV::draw() {
  if (minutes_view != NULL) {
    minutes_view->time = minutes;
    minutes_view->draw();
  }
  if (seconds_view != NULL) {
    seconds_view->time = seconds;
    seconds_view->draw();
  }
  if (colon_view != NULL)   colon_view->draw();
  return true;
}

// O.A.M.D.G