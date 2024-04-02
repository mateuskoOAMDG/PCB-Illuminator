/**************************************************************************
  Project: Osvitka s encoderom a OLED 128 x 32
  File   : Views.cpp
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

#include "Arduino.h"
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "Views.h"

extern Adafruit_SSD1306 display;

ViewContainer::ViewContainer(View* wfirstView, View* wlastView = NULL, View* wselected = NULL) {
  firstView = wfirstView;
  lastView = wlastView;
  selected = (wselected == NULL) ? wfirstView : wselected ;
}

void ViewContainer::doEvent(int event) {
  if (event == VIEW_EVENT_CLICK) {
    //selected->drawmode = mINVERSE | mCLEAR;
    selected->draw();
    if (selected->onClick != NULL) {
      selected->onClick();
      
    }
    //
  }

  else if (event == VIEW_EVENT_LONG) {
    //selected->drawmode = mINVERSE | mCLEAR;
    //selected->draw();
    if (selected->onLong != NULL) {
      selected->onLong();
    }
  }

  else if (event == VIEW_EVENT_PRESS) {
    //
  }

  else if (event == VIEW_EVENT_RELEASE) {
    //
  }

  else if (event > 0 && event <= 1000) {
    // encoder turns right
    if(selected == NULL) return;

    if(selected->mode & VIEW_MODE_EDITED) {
      if(selected->onTurn != NULL) selected->onTurn(event);
    }
    else {
      selected->drawmode = mCLEAR;
      selected->draw();
      View* temp = selected;
      bool next = true;
      while(next && (temp->nextView != NULL)) {
        temp = temp->nextView;
        if (!(temp->mode & VIEW_MODE_NOT_SELECTABLE)) next = false;
      } 
      if (temp != NULL) {
        if (!(temp->mode & VIEW_MODE_NOT_SELECTABLE)) selected = temp; 
      }
      selected->drawmode =mCLEAR | mFRAME;
    }
    selected->draw();

  }

  else if (event < 0 && event >= -1000) {
    // encoder turns right
    if(selected == NULL) return;
    if(selected->mode & VIEW_MODE_EDITED) {
      if(selected->onTurn != NULL) selected->onTurn(event);
    } 
    else { 

      selected->drawmode = mCLEAR;
      selected->draw();
      
      bool next = true;
      View* temp = selected;
      while(next && (temp->prevView != NULL)) {
        temp = temp->prevView;
        if (!(temp->mode & VIEW_MODE_NOT_SELECTABLE)) next = false;
      } 
      if (temp != NULL) {
         if (!(temp->mode & VIEW_MODE_NOT_SELECTABLE)) selected = temp; 
      }

      selected->drawmode = mCLEAR | mFRAME;
    }
    selected->draw();

  }
}

void ViewContainer::draw(byte xgroup) {
  View* item;
  item = firstView;
  while (item != NULL) {
    if (item == selected) {item->drawmode |= mFRAME;}
    if ((xgroup == 0) || (xgroup == item->group) )
    item->draw();
    if (item == lastView) 
      item = NULL;
    else
      item = item->nextView;
    
  }
}

void ViewContainer::forEach(byte xgroup, void(*action)(View* object_view)) {
  View* item;
  item = firstView;
  while (item != NULL) {
    if(item->group & xgroup) action(item);
    if (item == lastView) 
      item = NULL;
    else
      item = item->nextView;
    
  } 
}

void ViewContainer::setValue(byte group, byte value) {
  View* item;
  item = firstView;
  while (item != NULL) {
    if(item->group == group) item->value = value;
    if (item == lastView) 
      item = NULL;
    else 
      item = item->nextView;
    
  }
}

///////////////////////////// class View ////////////////////////////

View::View(byte x, byte y, byte w, byte h):x0(x), y0(y), w0(w), h0(h) {
  mode = VIEW_MODE_INACTIVE;
  drawmode = mCLEAR;
  nextView = NULL;
  prevView = NULL;
  group = 0;
  value = 0;
  onClick = NULL;
  onLong = NULL;
  onTurn = NULL;
}

void View::draw(byte xgroup) {

}


void View::setEdit(bool edit) {
  if (edit) { //sú editované minúty
    mode |= VIEW_MODE_EDITED;
    drawmode = mCLEAR | mINVERSE;
  }
  else {
    mode &= ~VIEW_MODE_EDITED;
    drawmode = mCLEAR | mFRAME;
  }
  draw();
  display.display();
};

//////////////////////////// class LabelView ///////////////////////////

void LabelView::draw(byte xgroup) {
  if (drawmode & mCLEAR) display.fillRect(x0, y0 - 16, w0, h0,  (drawmode & mINVERSE) ? SSD1306_WHITE : SSD1306_BLACK);
  if (drawmode & mFRAME) display.drawRect(x0, y0 - 16, w0, h0,  (drawmode & mINVERSE) ? SSD1306_BLACK : SSD1306_WHITE);
  
  if (drawmode & mHIDE) return;

  display.setFont(&FONT);
  display.setCursor(x0 + xTextOffset, y0);
  display.setTextColor(drawmode & mINVERSE ? SSD1306_BLACK : SSD1306_WHITE);
  display.print(text);
}


void LabelView::setText(__FlashStringHelper *xtext) {
  text = xtext;
}
//////////////////////////// class Time View ///////////////////////////

void TimeView::draw(byte xgroup) {
  // Draws number 00-99 at position [x0,y0] in mode 
  // if number < 0 - no action
  // mode - 0x01 - color White  (mWHITE) - foreground color bit - DEPRECATED
  //      - 0x02 - frame        mFRAME - frame bit (if background white frm is black etc...)
  //      - 0x04 - inverse      mINVERSE - background color bit
  //      - 0x08 - clear first  mCLEAR - clear before drawing bit(fillRect bgcolor)
 
  if (drawmode & mCLEAR) display.fillRect(x0, y0-16, 29, 19,  (drawmode & mINVERSE) ? SSD1306_WHITE : 0);
  if (drawmode & mHIDE) return;
  if (drawmode & mFRAME) display.drawRect(x0, y0-16, 29, 19,  (drawmode & mINVERSE) ? 0 : SSD1306_WHITE);
 

  if (time < 0) return; //no draw negative numbers
  display.setFont(& FONT);
  display.setCursor(x0, y0);
  display.setTextColor(drawmode & mINVERSE ? SSD1306_BLACK : SSD1306_WHITE);
  
  char character;
  character = time / 10 + '0';
  display.write(character);
  
  character = time % 10 + '0';
  display.write(character);
}

////////////////// class ButtonView /////////////////////////

void ButtonView::draw(byte xgroup) {

  if (value == 1) drawmode |= mINVERSE; else drawmode &= ~mINVERSE;

  if (drawmode & mCLEAR) display.fillRect(x0, y0, w0, h0, SSD1306_BLACK);
  if (drawmode & mINVERSE) display.fillRect(x0+2, y0+2, w0-4, h0-4, SSD1306_WHITE);  
  if (drawmode & mFRAME) display.drawRect(x0, y0, w0, h0, SSD1306_WHITE);
  
   display.setFont(NULL); 
   display.setCursor(x0 + (w0-5)/2, y0 + (h0-7)/2);
 
  display.setTextColor(drawmode & mINVERSE ?  SSD1306_BLACK : SSD1306_WHITE);
  display.write(ch);
}

/////////////////////////////////////////////////////

void IconView::draw(byte xgroup) {
  if (drawmode & mCLEAR) display.fillRect(x0, y0, w0, h0,  (drawmode & mINVERSE) ? SSD1306_WHITE : SSD1306_BLACK);
  if (drawmode & mFRAME) display.drawRect(x0, y0, w0, h0,  (drawmode & mINVERSE) ? SSD1306_BLACK : SSD1306_WHITE);

  display.drawXBitmap(
    x0,
    y0,
    icon_array, 28, 28, drawmode & mINVERSE ? 0 : 1);
}

void IconView::setIcon(const unsigned char *icon) {
  icon_array = icon;
}
