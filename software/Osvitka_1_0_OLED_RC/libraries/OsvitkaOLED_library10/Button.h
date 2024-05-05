/*
 * Nanovo naprogramovaná trieda DebounceButton a RealButton
 * Trieda zapuzdruje softwerovy debouncing pre tlacitko
 * optimalizovane na rychlost a spotrebu RAM pre instanciu tlacitka
 * 
 */

#ifndef BUTTON_H
#define BUTTON_H
#include "Arduino.h"

#define BUTTON_DEBOUNCE_TIME 20u //default cas debouncingu
#define BUTTON_LONG_TIME     1000u //default long time form long timer
#define BUTTON_VLONG_TIME    3000u //default vlong time form long timer

class DebounceButton {
    public:
        /// @brief Constructor + initialization
        /// @param pin Pin number
        /// @param mode Pin mode INPUT | INPUT_PULLUP (default)
        /// @param pressed Button "down" on state: LOW (default) | HIGH
        /// @param debouncetime time in ms (20ms default)
        DebounceButton(uint8_t pin, uint8_t mode = INPUT_PULLUP, bool down = LOW, uint16_t debouncetime = BUTTON_DEBOUNCE_TIME); 
        
        /// @brief Returns true if button "down"
        /// @return true (button is "pressed - down") | false (button is not "pressed -up")
        bool pressed();
        
        /// @brief initializing button (start debounce timer etc...)
        void start();

        /// @brief Update state of button. Musi byt volana castejsie ako je debounce time
        /// @return true - status of button changed | false - not changed
        bool update();

        /// @brief Time of debouncing
        uint16_t debounceTime;
    
    protected:
        
    
    
        uint8_t _pin; //pin number
        uint8_t _state; //bit0 - down state 0 - LOW, 1 - HIGH; bit1 - old pin level 0 - LOW, 1 - HIGH
        uint8_t _state2;//bit-0 OnLong readed
    private:
        uint16_t _debounce_timer; //internal debouncing timer
};

// parametre funkcie reset
#define BUTTON_RESET_ALL        0xFF
#define BUTTON_RESET_PRESS      0x01
#define BUTTON_RESET_RELEASE    0x02
#define BUTTON_RESET_LONG       0x04
#define BUTTON_RESET_VERY_LONG  0x08

#define BUTTON_TIMER_MIN_DOUBLE_TIME    60u //mimimal duration in ms - press/release for double click 
#define BUTTON_TIMER_MAX_DOUBLE_PRESS_TIME 200u
#define BUTTON_TIMER_MAX_DOUBLE_RELEASE_TIME 200u

class RealButton : public DebounceButton {
    public:

    uint16_t longTime;
    uint16_t veryLongTime;
    uint16_t dblPressTime;
    uint16_t dblReleaseTime;
    

    /// @brief Constructor + initialization
    /// @param pin Pin number
    /// @param mode Pin mode INPUT | INPUT_PULLUP (default)
    /// @param pressed Button "down" on state: LOW (default) | HIGH
    /// @param debouncetime time in ms (20ms default)
    RealButton(uint8_t pin, uint8_t mode = INPUT_PULLUP, bool down = LOW, uint16_t debouncetime = BUTTON_DEBOUNCE_TIME);
    
    
    /// @brief inicializuje tlacitko
    /// @param down pociatocna inicializacia true - tlacitko stlacene, false - tlacitko nestlacene 
    void start(bool down = false);
    
    /// @brief Udalost sa generuje: V okamihu stlacenia tlacidla
    /// @return true - bolo prave stlacene
    bool onPress(bool reset = true);

    /// @brief Udalost sa generuje: V okamihu uvolnenia tlacidla
    /// @return true - bolo prave uvolnene
    bool onRelease(bool reset = true);

    /// @brief Udalost sa generuje: Pri uvolneni tlacidla v case kliku (nie dlhsom ako longTime)
    /// @return true - bolo kratke stlacenie tlacidla (generuje sa pri uvolneni)
    bool onClick(bool reset = true); 

    /// @brief Udalost sa generuje: V okamihu prekrocenia casu "longTime"
    /// @return  true - tlacitko bolo stlacene dlhsie ako longTime
    bool onLong(bool reset = true);

    /// @brief Udalost sa generuje: Pri uvolneni tlacidla v case dlheho kliku (nie dlhsom ako veryLongTime)
    /// @return true - bolo dlhe stlacenie tlacitka (generuje sa pri uvolneni)
    bool onLongClick(bool reset = true);

    /// @brief Udalost sa generuje: V okamihu prekrocenia casu "veryLongTime"
    /// @return true - tlacitko bolo stlacene dlhsie ako veryLongTime
    bool onVLong(bool reset = true);

    /// @brief Udalost sa generuje: Pri uvolneni tlacidla v case velmi dlheho kliku (horna hranica casu je 30s!)
    /// @return true - bolo dlhe stlacenie tlacitka (generuje sa pri uvolneni)
    bool onVLongClick(bool reset = true);

    /// @brief Udalost sa generuje: Pri dvojkliku tlacidla
    /// @return true - Po dvojkliku (dvojklik je definovany casmi dblpressTime, dblreleaseTime)
    bool onDouble(bool reset = true);

    /// @brief Updatovanie stavu tlacitka. Udalostne metody len vracaju stavy, ktore sa nastavili v tejto procedure
    /// @return 
    bool update();
    
    /// @brief Reset status
    /// @param what what is reset
    /// @return NC
    bool reset(uint8_t what = BUTTON_RESET_ALL);
    
    /// @brief Returns debouncing progress status
    /// @return true - debouncing in progress, false - done, is ok value
    bool isDebouncing(); //prebieha debouncing

    /// @brief Returns double-click progress status
    /// @return true - double click analyzing in progres0s, false - idle
    bool isUpdateDouble(); //prebieha spracovanie dvojkliku
 private:
    uint16_t _longTimer; //timer pre "long" a "veryLong"
    uint16_t _dblTimer;  //timer pre doubleClick
    uint8_t  _realState; //statusy jednotlivych udalosti
    
    bool _oldPressed;     //predosly stav pressed
    uint8_t _timerState;  //statusy zapnutia timerov b0 = _longTimer running, b1=_dblTimers running
    uint8_t qn_double;    //cislo stavu pre dvojklik (stavy automatu)

    bool updateDouble();  //Updates progress for double-click; this is called from update();

};

#endif
