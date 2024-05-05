/**************************************************************************
  Illuminator PCB with Countdown Timer. 
  LCD 16x2 & Encoder with Switch. 
  Arduino project.
  
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Illuminator PCB with Encoder & LCD 16 x 2
  File   : Views.h
  MCU    : ATMEGA328P
  
  
  Version: view2.01
  Date   : 2024-05-03
  Author : mateusko.OAMDG@outlook.com

  Definition of classes for drawing on display.
  View         - base class
  TimeView     - class for display numbers 00-99
  ButtonView   - class for display preset buttons
  LabelView    - class for display text

  ViewContainer - container for views (draw screen on display)
 **************************************************************************/

#ifndef _VIEWS_H_
#define _VIEWS_H_



// Constants for View instances draw mode. The instance ...
#define VIEW_MODE_ENABLED     0x01 // ... is enabled now
#define VIEW_MODE_SELECTED    0x02 // ... is selected now
#define VIEW_MODE_EDITED      0x04 // ... is edited now
#define VIEW_MODE_VISIBLE     0x10 // ... is visible
#define VIEW_MODE_SELECTABLE  0x20 // can by selected 
#define VIEW_MODE_EDITABLE    0x40 // can by edited
#define VIEW_MODE_CLEAR       0x08 // draw only empty-area (space chars)

#define VIEW_MODE_LABEL       (VIEW_MODE_VISIBLE | VIEW_MODE_ENABLED)
#define VIEW_MODE_BUTTON      (VIEW_MODE_VISIBLE | VIEW_MODE_ENABLED | VIEW_MODE_SELECTABLE)
#define VIEW_MODE_TIME        (VIEW_MODE_VISIBLE | VIEW_MODE_ENABLED | VIEW_MODE_SELECTABLE | VIEW_MODE_EDITABLE)

// Event codes
#define VIEW_EVENT_CLICK   10000
#define VIEW_EVENT_LONG    10001
#define VIEW_EVENT_PRESS   10002
#define VIEW_EVENT_RELEASE 10003
  //  1, -1  rotation Impuls R or L


///////////////////// CLASS VIEW /////////////////

// Base class for View objects
class View {
  public:
    View(byte x, byte y, byte width); //constructor
      // selector-char is not counted in the width of the object 

    byte x0;    //position on display - x horizontal
    byte y0;    //position on display - y vertical
    byte w0;    //width - horizontal size without selector-chars
    byte mode;  // view mode - see constats VIEW_MODE_...
    
    byte group; // auxiliary variable, e.g. switch group, instance identifier etc...
    byte value; // variable for different purposes
    
    View* nextView; //linked list of Views - next instance
    View* prevView; //linked list of Views - previous instance
    

    //callback functions
    void (*onClick)(); // calls if selected; if is NULL -> to edit mode; if edit mode -> end edit mode
    void (*onLong)();  // calls if selected
    void (*onTurn)(int event); // if View is edited, events are sent to this function

    
    // "mode" getters and setters
    inline bool isEdited()    {return mode & VIEW_MODE_EDITED;};
    inline bool isSelected()  {return mode & VIEW_MODE_SELECTED;}
    inline bool isVisible()   {return mode & VIEW_MODE_VISIBLE;}
    inline bool isSelectable(){return mode & VIEW_MODE_SELECTABLE;}
    inline bool isEditable()  {return mode & VIEW_MODE_EDITABLE;}
    inline bool isClear()     {return mode & VIEW_MODE_CLEAR;}
    
    inline void setVisible(bool visible) {
      if (visible) 
        mode |= VIEW_MODE_VISIBLE;
      else
        mode &= ~VIEW_MODE_VISIBLE;
    }
    
    inline void setSelected(bool selected) {
      if (selected) 
        mode |= VIEW_MODE_SELECTED;
      else
        mode &= ~VIEW_MODE_SELECTED;
    }

    inline void setSelectable(bool selectable) {
      if (selectable) 
        mode |= VIEW_MODE_SELECTABLE;
      else
        mode &= ~VIEW_MODE_SELECTABLE;
    }
    
    inline void setEdited(bool edited) {
      if (edited) 
        mode |= VIEW_MODE_EDITED;
      else
        mode &= ~VIEW_MODE_EDITED;
    }

    inline void setClear(bool clear) {
      if (clear) 
        mode |= VIEW_MODE_CLEAR;
      else
        mode &= ~VIEW_MODE_CLEAR;
    }

    virtual void draw(byte xgroup = 0); //displays View
    void clear(); //deletes the characters in the View position on the display
    virtual void drawSelector(bool xdraw); // displays selector-chars < > around the View
    virtual void drawEditor(bool xdraw);   // displays editor-chars * * around the View
};

///////////////////// CLASS LabelView /////////////////

// class to display the text
class LabelView: public View {
  public:
  String text;
   LabelView(byte x, byte y, byte w, String xtext);
   void draw(byte xgroup = 0);
};

///////////////////// CLASS TimeView /////////////////

// class to display the numbers from 00 to 99
class TimeView: public View {
  public:
   int time;
   TimeView(byte x, byte y);
   void draw(byte xgroup = 0);
};

///////////////////// CLASS ButtonView /////////////////

// class to display a button with text
class ButtonView: public View {
  public:
  String text;
   ButtonView(byte x, byte y, byte w, String xtext);
   void draw(byte xgroup = 0);
};

///////////////////// CLASS ViewContainer /////////////////

// Container class for View objects. 
// Enables drawing and event processing of Views objects
class ViewContainer {
  public:
    ViewContainer();
    View* firstView; // pointers to 1. View
    View* lastView;  // pointer to the last View
    View* selected;  // pointer to the Selected View 
    bool editing;    // flag that the object is currently being edited

    void draw(byte xgroup = 0, bool clear = true); //renders all Views from the container to the display
    void doEvent(int event); // processes the incoming event

    void add(View& view); //adds View object to the end of ViewContainer list   
    
    void setSelected(View& wselected);
     
    /* it is currently not implemented
    void setValue(byte group, byte value);
    void forEach(byte xgroup, void(*action)(View* object_view));
    */
};


#endif
