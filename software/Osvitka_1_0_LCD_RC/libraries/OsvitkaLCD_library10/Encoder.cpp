/**************************************************************************
  Illuminator PCB with Countdown Timer. 
  LCD 16x2 & Encoder with Switch. 
  Arduino project.
  
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Illuminator PCB with Encoder & LCD 16 x 2
  File   : Encoder.cpp
  MCU    : ATMEGA328P
   
  Version: enc.0.02
  Date   : 2024-05-03
  Author : mateusko.OAMDG@outlook.com

  Rotary Encoder support uses interrupts on pins 2 and 3. Only for ATMEGA328P (see *.cpp for other)

 **************************************************************************/

#include "Arduino.h"
#include "Encoder.h"


//internal 
volatile bool _enc_A;
volatile bool _enc_B;
volatile bool _enc_isvalue = false;
volatile byte _enc_temp = 0;
volatile int _enc_counter = 0;



// Interrupt function 
void ISR_encoder() {
  bool a,b;
  
  a = PIND & 0x04;
  b = PIND & 0x08;
  
  //filter: ak sa nacitajhu rovnake hodnoty ako predtym - return
  if ((a == _enc_A) && (b == _enc_B)) return;

  //filter: ak je zhoda, caka sa na rozdiel

  if ((_enc_A == _enc_B) && (a == b)) return;

  //filter: ak je rozdiel, caka sa zhoda
  if ((_enc_A != _enc_B) && (a != b)) return;
  _enc_temp <<= 1;
  if (a) _enc_temp |= 1;
  _enc_temp <<= 1;
  if (b) _enc_temp |= 1;
  _enc_A = a;
  _enc_B = b;
  if(a & b) {
    if(_enc_temp == 0x4B) {
      _enc_counter ++;
      _enc_isvalue = true;
    }
    else if (_enc_temp == 0x87) {
      _enc_counter --;
      _enc_isvalue = true;
    }
  } 
}


Encoder::Encoder() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attach();
  direction = 1;
}

void Encoder::setDirection(bool vdirection) {
  direction = vdirection;
}

void Encoder::attach(){
  attachInterrupt(digitalPinToInterrupt(2), ISR_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), ISR_encoder, CHANGE);
}

void  Encoder::detach() {
  detachInterrupt(digitalPinToInterrupt(2));
  detachInterrupt(digitalPinToInterrupt(3));
}

 void Encoder::reset() {
  _enc_counter = 0;
 }

 int Encoder::impuls() {
   // read rotary state encoder
  // return:  +1 -1 0
  if (_enc_counter > 0) {
    _enc_counter --;
    return direction ? 1 : -1;
  }

  else if (_enc_counter < 0) {
    _enc_counter ++;
    return direction ? -1 : 1;
  }

  else
    return 0;

}
