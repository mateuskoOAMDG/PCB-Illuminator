/**************************************************************************
  
  Illuminator PCB with count down timer. Arduino project.
  Copyright (C) 2024 mateusko O.A.M.D.G

  
  
  Project: Osvitka s encoderom a OLED 128 x 32
  File   : Encoder.h
  MCU    : ATMEGA328P
  PCB    : (PINOUT) 1.0.2 rev.12
  
  Version: enc.1.01
  Date   : 2024-03-31
  Author : mateusko.OAMDG@outlook.com

  Encoder rotation support using an interrupt on pins 2 and 3.

 **************************************************************************/

/*
 INTERFACE:
    void encoder_init();
      must called in setup();

    int encoder_impuls();
      return -1 +1 0
    */

#ifndef _ENCODER_H_
#define  _ENCODER_H_

// Rotary Encoder Interrupt variables
volatile bool _enc_A;
volatile bool _enc_B;
volatile bool _enc_isvalue = false;
volatile byte _enc_temp = 0;
volatile int _enc_counter = 0;

/// @brief Interrupt routine
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


void attachIntEncoder(){
  attachInterrupt(digitalPinToInterrupt(ENC_A), ISR_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), ISR_encoder, CHANGE);
}

void detachIntEncoder() {
  detachInterrupt(digitalPinToInterrupt(ENC_A));
  detachInterrupt(digitalPinToInterrupt(ENC_B));
}

/// @brief Init encoder. Must by called from setup()
void encoder_init() {
  // Encoder initialisation
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);

    attachIntEncoder();
}

/// @brief Reads the encoder rotation state
/// @return +i R, -i L, 0 no rotation; i - number of step.
int encoder_impuls() {
  // read rotary state encoder
  // return:  +1 -1 0
  if (_enc_counter > 0) {
    _enc_counter --;
    return 1;
  }

  else if (_enc_counter < 0) {
    _enc_counter ++;
    return -1;
  }

  else
    return 0;

}

#endif

//O.A.M.D.G
