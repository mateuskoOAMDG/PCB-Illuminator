/**************************************************************************
  Illuminator PCB with count down timer. Arduino project.
  Copyright (C) 2024 mateusko O.A.M.D.G

  
  
  Project: Osvitka s encoderom a OLED 128 x 32
  File   : Views.h
  MCU    : ATMEGA328P
  
  
  Version: view.02
  Date   : 2024-03-31
  Author : mateusko.OAMDG@outlook.com

  Definition of classes for drawing on display.
  View         - base class
  TimeView     - class for display numbers 00-99
  ButtonView   - class for display preset buttons
  IconView     - class for display button with icon
  LabelView    - class for display text

  ViewContainer - container for views (draw screen on display)
 **************************************************************************/

#ifndef _VIEWS_H_
#define _VIEWS_H_


#define mFRAME   0x02 // draw frame (else NO frame)
#define mINVERSE 0x04 // draw inverse BLACK numbers on WHITE background
#define mCLEAR   0x08 // clear drawarea before drawing
#define mHIDE    0x10 // no draw; only empty area

#define FONT FreeMonoBold12pt7b

#define VIEW_MODE_INACTIVE 0 
#define VIEW_MODE_SELECTED 1
#define VIEW_MODE_EDITED   2
#define VIEW_MODE_HIDE     4
#define VIEW_MODE_NOT_SELECTABLE 8 //

#define VIEW_EVENT_CLICK   10000
#define VIEW_EVENT_LONG    10001
#define VIEW_EVENT_PRESS   10002
#define VIEW_EVENT_RELEASE 10003
//-1000 .. 1000 TURN R+L


#include "icons.h"

class View {
  public:
    View(byte x, byte y, byte w, byte h); //constructor
    byte x0; //position on display - x horizontal
    byte y0; //position on display - y vertical
    byte w0; //width - horizontal size
    byte h0; //height - vertical size
    byte mode; // view mode - see constats #define VIEW_MODE_...
    byte drawmode;
    View* nextView;
    byte group; //pomocná variable, napr. skupina prepínačov, identifikátor inštancie etc...
    byte value; //hodnota na rôzne účely, pri ButtonView - vybraté tlačidlo
    View* prevView;
    

    //callback function
    void (*onClick)(); // calls if selected; if is NULL -> to edit mode; if edit mode -> end edit mode
    void (*onLong)();  // calls if selected
    void (*onTurn)(int count); // calls if edited
    
    inline bool isEdit() {return mode & VIEW_MODE_EDITED;};
    void setEdit(bool edit);

    virtual void draw(byte xgroup = 0);
};

class LabelView: public View {
  public:
     char xTextOffset;
    __FlashStringHelper* text;
    LabelView(byte x, byte y, byte w, byte h) : View(x , y, w, h) , xTextOffset(0) {
        
        mode = VIEW_MODE_NOT_SELECTABLE; //neda sa vybrať
    }
    void setText(__FlashStringHelper *xtext);
    void draw(byte xgroup = 0);
};

class TimeView: public View {
  public:
   int time;
   TimeView(byte x, byte y, byte w, byte h) : View(x ,y, w, h) {
    value = 0;
   };
   
   void draw(byte xgroup = 0);
};

class ButtonView: public View {
  public:
   char ch;
   ButtonView(byte x, byte y, byte w, byte h, char character) : View(x, y, w, h), ch(character) {};

   void draw(byte xgroup = 0);
};

class IconView: public View {
  public:
   const unsigned char *icon_array;
   IconView(byte x, byte y, const unsigned char *icon): View(x, y, 28, 28), icon_array(icon) {};
   void setIcon(const unsigned char *icon);
   void draw(byte xgroup = 0);
};


class ViewContainer {
  public:
    ViewContainer(View* wfirstView, View* wlastView = NULL, View* wselected = NULL);
    View* firstView; //pointers on 1. View
    View* lastView;
    View* selected;

    void draw(byte xgroup = 0);
    void doEvent(int event);
    void setValue(byte group, byte value);
    void forEach(byte xgroup, void(*action)(View* object_view));
};


#endif
