/**************************************************************************
  Illuminator PCB with Countdown Timer. 
  LCD 16x2 & Encoder with Switch. 
  Arduino project.
  
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Illuminator PCB with Encoder & LCD 16 x 2
  File   : Encoder.h
  MCU    : ATMEGA328P
   
  Version: enc.0.02
  Date   : 2024-05-03
  Author : mateusko.OAMDG@outlook.com

  Rotary Encoder support uses interrupts on pins 2 and 3. Only for ATMEGA328P (see *.cpp for other)
  
  This version of the encoder handler (via function impuls()) 
  returns separate pulses 
    -1  - if turning left, 
    1   - if turning right 
    0 - if not turning
 **************************************************************************/


#ifndef _ENCODER_H_
#define  _ENCODER_H_

class Encoder {
  public:
    Encoder();
    void attach();    // attach Encoder interrupt
    void detach();    // detach Encoder interrupt
    void setDirection(bool direction); // set turn direction. Value +1 or -1
    void reset();     //reset internal counter
    int impuls();     //returns one impuls (-1, 1 or 0 if no rotation)
   
  private:
    bool direction;
    
};

#endif

//O.A.M.D.G
