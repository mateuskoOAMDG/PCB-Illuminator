/**************************************************************************
  Illuminator PCB with count down timer. Arduino project.
  Copyright (C) 2024 mateusko O.A.M.D.G

  
  Project: Osvitka s encoderom a OLED 128 x 32
  File   : TimerV.h
  MCU    : ATMEGA328P
  
  
  Version: tmrv 0.02
  Date   : 2024-03-31
  Author : mateusko.OAMDG@outlook.com

  Definition of classes for count down Timer


 **************************************************************************/

#ifndef _TIMERV_H_
#define _TIMERV_H_
#include "Views.h"

#define TIMERV_BLINK_COLON_TIME 500u //milsec
#define TIMERV_SPEED_BLINK_COLON 50u //milsec
#define TIMERV_SECOND_TIME 1000u //milsec

//internal
#define TIMERV_MODE_STOP 0
#define TIMERV_MODE_START 1
#define TIMERV_MODE_PAUSE 2

/// @brief Cont Down Timer class
class TimerV { 
  public:
    TimerV();
    void setView(TimeView* wminutes, TimeView* wseconds, LabelView* wcolon); // Assign View instances
    void start(); //start countdown
    void stop();  //stop countdown 
    void pause(); //pause countdown
    bool addMinutes(int count); //add minutes +-
    bool addSeconds(int count); //add minutes +-
    bool setTime(byte xminutes, byte xseconds); //set time
    inline bool isZero() {return (minutes == 0) && (seconds == 0);}; // returns "true" if time is 00:00
    void update(); // updates time and blink colon; must by called regularly in loop
    bool draw(); // it draws timer on display via View instances
  public:
    int minutes;
    int seconds;
    int mode;
    TimeView* minutes_view; // View Instance for display minutes
    TimeView* seconds_view; // View Instance for display seconds
    LabelView* colon_view;  // View Instance for display colon

  private:
    unsigned int timer; // second tick timer

};

#endif