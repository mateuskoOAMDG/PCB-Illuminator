#include "RealButton.h"

#define BUTTON_STATE_DOWN_BIT 0 // kedy je tlacidlo stlacene 0 = LOW | 1 = HIGH
#define BUTTON_STATE_OLD_LEVEL_BIT 1 //predosly level pinu 0 = LOW | 1 = HIGH
#define BUTTON_STATE_PRESSED_BIT 2   //aktualny stav pressed 0 = "UP" | 1 = "DOWN"
#define BUTTON_STATE_AFTER_DEBOUNCE_BIT 3 //internal 0 = on debounce | 1 

/////////////// DebounceButton ///////////////

DebounceButton::DebounceButton(uint8_t pin, uint8_t pin_mode = INPUT_PULLUP, bool down = LOW, uint16_t debounce_time = 20u) {
    _pin = pin; //set pin number
    pinMode(pin, pin_mode); //set pin mode
    
    _state = (down == HIGH) ? _BV(BUTTON_STATE_DOWN_BIT) : 0;
    
    debounceTime = debounce_time; //set debounce time
    start();
}


void DebounceButton::start() {
    _debounce_timer = millis();
    if (digitalRead(_pin) == HIGH) _state |= _BV(BUTTON_STATE_OLD_LEVEL_BIT);
}

bool DebounceButton::pressed() {
    return _state & _BV(BUTTON_STATE_PRESSED_BIT);
}

bool DebounceButton::update() {
    if (digitalRead(_pin) == HIGH) {
        if (_state & _BV(BUTTON_STATE_OLD_LEVEL_BIT)) {
            
            //bolo HIGH a teraz je HIGH
            if (_state & _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT)) return false;
            
            if (uint16_t(millis()) - _debounce_timer > debounceTime) {
                _state |= _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT); //prave sa dokoncil debouncing 
                
                if(_state & _BV(BUTTON_STATE_DOWN_BIT)) // nastavenie statusu DOWN
                    _state |= _BV(BUTTON_STATE_PRESSED_BIT);
                else
                    _state &= ~_BV(BUTTON_STATE_PRESSED_BIT);
            }
            return true; //vratenie priznaku, ze prebieha debouncing

        } else {
            //bolo LOW a teraz je HIGH
            _debounce_timer = millis(); //restart debouncing timera
            _state &= ~ _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT); //nastavenie statusu, ze prebieha debouncing
            _state |= _BV(BUTTON_STATE_OLD_LEVEL_BIT); //set BUTTON_STATE_OLD_LEVEL_BIT na HIGH
            return true; //prebieha debouncing
        }
    } else {
        if (_state & _BV(BUTTON_STATE_OLD_LEVEL_BIT)) { 
            // bolo HIGH a teraz je LOW
            _debounce_timer = millis(); //restart debouncing timera
            _state &= ~ _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT); //nastavenie statusu, ze prebieha debouncing
            _state &= ~_BV(BUTTON_STATE_OLD_LEVEL_BIT); //clear BUTTON_STATE_OLD_LEVEL_BIT (na LOW)
            return true; //prebieha debouncing

        } else {
            // bolo LOW a teraz je LOW
            if (_state & _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT)) return false;

            if (uint16_t(millis()) - _debounce_timer > debounceTime) {
                _state |= _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT); //prave sa dokoncil debouncing 
                
                if(_state & _BV(BUTTON_STATE_DOWN_BIT)) // nastavenie statusu DOWN
                    _state &= ~_BV(BUTTON_STATE_PRESSED_BIT);
                else
                    _state |= _BV(BUTTON_STATE_PRESSED_BIT);                   
            }
            return true;
            
        }       
    }
}

////////////////// RealButton //////////////////

// _real_state
#define BUTTON_STATE_ON_PRESS_BP        0
#define BUTTON_STATE_ON_RELEASE_BP      1
#define BUTTON_STATE_ON_LONG_BP         2
#define BUTTON_STATE_ON_VLONG_BP        3
#define BUTTON_STATE_ON_CLICK_BP        4
#define BUTTON_STATE_ON_DOUBLE_BP       5
#define BUTTON_STATE_ON_LONGCLICK_BP    6
#define BUTTON_STATE_ON_VLONGCLICK_BP   7

// _real_state2
#define BUTTON_STATE2_ON_LONG_READED_BP    2 //ci bola long precitana (aby sa negenerovala viackrat)
#define BUTTON_STATE2_ON_VLONG_READED_BP   3 //ci bola vlong precitana (aby sa negenerovala viackrat)



#define BUTTON_TIMER_STATE_LONG_BP      0
#define BUTTON_TIMER_STATE_DOUBLE_BP    1


RealButton::RealButton(uint8_t pin, uint8_t mode, bool down, uint16_t debouncetime) : DebounceButton(pin, mode, down, debouncetime) {
    _realState = 0;
    _realState2 = 0;
    _timerState = 0;
    _oldPressed = false;
    autoreset = true; //automatické vymazanie priznaku long po release()
    longTime = BUTTON_LONG_TIME;
    veryLongTime = BUTTON_VLONG_TIME;
    dblPressTime = BUTTON_TIMER_MAX_DOUBLE_PRESS_TIME;
    dblReleaseTime = BUTTON_TIMER_MAX_DOUBLE_RELEASE_TIME;
    DebounceButton::start();
}


void RealButton::start(bool down) {
    reset();
    update();
    _oldPressed = down;

}

bool RealButton::onPress(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_PRESS_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_PRESS_BP);
    return value;
}

bool RealButton::onRelease(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_RELEASE_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_RELEASE_BP);
    return value;
}

bool RealButton::onClick(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_CLICK_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_CLICK_BP);
    return value;
}

bool RealButton::onLong() {
    bool value = _realState & _BV(BUTTON_STATE_ON_LONG_BP);
    if (value) { //ak uz nastalo Long, tak skontroluj, ci uz bola udalost precitana
        value = value && !(_realState2 & _BV(BUTTON_STATE2_ON_LONG_READED_BP));
        _realState2 |= _BV(BUTTON_STATE2_ON_LONG_READED_BP);
    }
    return value;
}

void RealButton::longReset() {
    _realState &= ~_BV(BUTTON_STATE_ON_LONG_BP);
    _realState2 &=  ~_BV(BUTTON_STATE2_ON_LONG_READED_BP);
}

bool RealButton::onLongClick(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_LONGCLICK_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_LONGCLICK_BP);
    return value;
}

// tuto metodu treba este upravit podla onLong()!!!
bool RealButton::onVLong(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_VLONG_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_VLONG_BP);
    return value;
}

bool RealButton::onVLongClick(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_VLONGCLICK_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_VLONGCLICK_BP);
    return value;
}

bool RealButton::onDouble(bool reset) {
    bool value = _realState & _BV(BUTTON_STATE_ON_DOUBLE_BP);
    if(reset) _realState &= ~_BV(BUTTON_STATE_ON_DOUBLE_BP);
    return value;
}

bool  RealButton::reset(uint8_t what) {
    if (what == BUTTON_RESET_ALL) {
        _longTimer = 0;
        _dblTimer = 0;
        _realState = 0;
        _timerState = 0;
        DebounceButton::start();
    }
}

bool RealButton::isDebouncing() {
    return !(_state & _BV(BUTTON_STATE_AFTER_DEBOUNCE_BIT));
}

bool RealButton::update() {
    DebounceButton::update();
    if (_oldPressed) { //predtym BOLO STLACENE
        
        if (pressed()) {     //a stale JE STLACENE
            if (_timerState & _BV(BUTTON_TIMER_STATE_LONG_BP)) {  //je zapnuty timer long
                if (uint16_t(millis()) - _longTimer > longTime) //ak je DLHO STLACENE
                    _realState |= _BV(BUTTON_STATE_ON_LONG_BP);

                if (uint16_t(millis()) - _longTimer > veryLongTime) //ak je VELMI DLHO STLACENE
                    _realState |= _BV(BUTTON_STATE_ON_VLONG_BP);
            }
                 
        } else {             //a uz NIE JE STLACENE
            if(autoreset) {
                _realState2 &= ~_BV(BUTTON_STATE2_ON_LONG_READED_BP);
                _realState &= ~_BV(BUTTON_STATE_ON_LONG_BP);
            }
            _realState |= _BV(BUTTON_STATE_ON_RELEASE_BP); //nastav "on release"
            
            if (_timerState & _BV(BUTTON_TIMER_STATE_LONG_BP)) { //ak je zapnuty timer long
                if (uint16_t(millis()) - _longTimer < longTime) 
                    // ak sa uvolnilo pred logTime nastavi sa udalost ON_CLICK
                    _realState |= _BV(BUTTON_STATE_ON_CLICK_BP);
                    
                else if (uint16_t(millis()) - _longTimer < veryLongTime) 
                    // ak sa uvolnilo pred verylogTime nastavi sa udalost ON_LONGCLICK
                    _realState |= _BV(BUTTON_STATE_ON_LONGCLICK_BP);
                
                else 
                    // nastavi sa ON_VLONGCLICK
                    _realState |= _BV(BUTTON_STATE_ON_VLONGCLICK_BP);
            }
                
            _timerState &= ~ _BV(BUTTON_TIMER_STATE_LONG_BP); //vypni timer
            _oldPressed = false; //nastav old pressed na UVOLNENE
        }
    } else {          // predtym NEBOLO STLACENE
        if (pressed()) { // a teraz JE STLACENE
            _realState |= _BV(BUTTON_STATE_ON_PRESS_BP); //nastav "on press"
            _timerState |= _BV(BUTTON_TIMER_STATE_LONG_BP); //zapni timer long
            _longTimer = millis();
            _oldPressed = true;
        }
    }
    updateDouble();
}



bool RealButton::updateDouble() {
    uint16_t deltaT = uint16_t(millis()) - _dblTimer;
    switch (qn_double) {
        case 0:
            if(pressed()) {
               // _timerState |= _BV(BUTTON_TIMER_STATE_DOUBLE_BP);
                _dblTimer = millis();
                qn_double = 1;
            }
            break;
        case 1:
            if (deltaT > dblPressTime) qn_double = 0;
            else if (!pressed()  && deltaT > BUTTON_TIMER_MIN_DOUBLE_TIME) { 
                qn_double = 2;
                _dblTimer = millis();
            }
            break;
        case 2: 
            if (deltaT > dblReleaseTime) qn_double = 0;
            else if  (pressed() &&  deltaT > BUTTON_TIMER_MIN_DOUBLE_TIME) {
                 qn_double = 3;
                _dblTimer = millis();
            }
            break;
        case 3:
            if (deltaT > dblPressTime) qn_double = 0;
            else if (!pressed()  && deltaT > BUTTON_TIMER_MIN_DOUBLE_TIME) { 
                qn_double = 0;
                _dblTimer = 0;
                _timerState &= ~ _BV(BUTTON_TIMER_STATE_DOUBLE_BP);
                _realState |= _BV(BUTTON_STATE_ON_DOUBLE_BP);
            }
            break;
    }
    return qn_double;
}

bool RealButton::isUpdateDouble() {
    return qn_double;
}