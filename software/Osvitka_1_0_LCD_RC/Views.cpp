/**************************************************************************
  Illuminator PCB with Countdown Timer. 
  LCD 16x2 & Encoder with Switch. 
  Arduino project.
  
  Copyright (C) 2024 mateusko O.A.M.D.G
  
  
  Project: Illuminator PCB with Encoder & LCD 16 x 2
  File   : Views.cpp
  MCU    : ATMEGA328P
  
  
  Version: view2.01
  Date   : 2024-05-03
  Author : mateusko.OAMDG@outlook.com

  Definition of classes for drawing on display.
  View         - base class
  TimeView     - class for display numbers 00-99
  ButtonView   - class for display buttons
  LabelView    - class for display text

  ViewContainer - container for views (draws Views on display and processes events)
 **************************************************************************/

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "Views.h"

extern LiquidCrystal_I2C lcd;


ViewContainer::ViewContainer() {
  firstView = NULL;
  lastView = NULL;
  selected = NULL;
  editing = false;
}

void ViewContainer::add(View& view) {
  if (firstView == NULL) { // if first view
    firstView = &view;
    lastView = &view;
    view.nextView = NULL;
    view.prevView = NULL;
  }
  else { //if next view
    lastView->nextView = &view;
    view.prevView = lastView;
    view.nextView = NULL;
    lastView = &view;
  }
}

void ViewContainer::setSelected(View& wselected) {
  selected =  &wselected;
}

void ViewContainer::doEvent(int event) {
  if (!event) return;
  
  if (editing && event != VIEW_EVENT_CLICK ) {
    if (selected->onTurn != NULL) selected->onTurn(event);
    return;
  }

  if (event == VIEW_EVENT_CLICK) { // Do View(EVENT_CLICK) if callback function defined
    selected->draw();
    if (selected->onClick != NULL) {
      selected->onClick();  
    } 
    else if (editing) { //end of edit
      editing = false;
    }
    else if (selected->isEditable()){
      editing = true;
    }

  }

  else if (event == VIEW_EVENT_LONG) { // Do View(EVENT_LONG_CLICK) if callback function defined
    if (selected->onLong != NULL) {
      selected->onLong();
    }
  }

  else if (event == VIEW_EVENT_PRESS) { //temporary not implemented
    //
  }

  else if (event == VIEW_EVENT_RELEASE) {
    //
  }
  
  else if (event > 0 && event <= 1000) {
    // encoder turns right
    
    if(selected == NULL) return;

    if(selected->isEdited()) { //if edited, send event to destination View
      if(selected->onTurn != NULL) selected->onTurn(event);
    }
    else { 
      //if not edited, select next View
      selected->drawSelector(false);
      
      View* temp = selected;
      bool next = true;
      while(next && (temp->nextView != NULL)) {
        temp = temp->nextView;
        if (temp->isSelectable() && temp->isVisible()) next = false;
      } 

      if (temp != NULL) {
        if (temp->isSelectable() && temp->isVisible()) selected = temp; 
      }
      
    }

    

  }

  else if (event < 0 && event >= -1000) {
    // encoder turns right
      selected->drawSelector(false);
      
      View* temp = selected;
      bool next = true;
      while(next && (temp->prevView != NULL)) {
        temp = temp->prevView;
        if (temp->isSelectable() && temp->isVisible()) next = false;
      } 

      if (temp != NULL) {
        if (temp->isSelectable() && temp->isVisible()) selected = temp; 
      }
      
    }

  draw(0,false);
  if (editing) 
    selected->drawEditor(true); 
  else
    selected->drawSelector(true);
  
}

void ViewContainer::draw(byte xgroup, bool clear) {
  View* item;
  item = firstView;
  bool bselected = false;
  if (clear) lcd.clear();
  while (item != NULL) {
   
    if ((xgroup == 0) || (xgroup == item->group)) {
      item->draw(); 
      if (item == selected) bselected = true; 
    }
    
    if (item == lastView) 
      item = NULL;
    else
      item = item->nextView;
    
  }
  if (bselected) selected->drawSelector(true);
}
/*
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
*/
/*
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


*/

///////////////////////////// class View ////////////////////////////

View::View(byte x, byte y, byte width):x0(x), y0(y), w0(width) {
  mode = VIEW_MODE_ENABLED | VIEW_MODE_VISIBLE;

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

void  View::clear() {
  lcd.setCursor(x0, y0);
  for (int i = 0; i < w0; i++) lcd.write(' ');

}

void  View::drawSelector(bool xdraw) {
  if (xdraw) {
    lcd.setCursor(x0 - 1, y0);
    lcd.print('<');
    lcd.setCursor(x0 + w0, y0);
    lcd.print('>');
  }
  else {
    lcd.setCursor(x0 - 1, y0);
    lcd.print(' ');
    lcd.setCursor(x0 + w0, y0);
    lcd.print(' ');
  }
}

void  View::drawEditor(bool xdraw) {
  if (xdraw) {
    lcd.setCursor(x0 - 1, y0);
    lcd.print('*');
    lcd.setCursor(x0 + w0, y0);
    lcd.print('*');
  }
  else {
    lcd.setCursor(x0 - 1, y0);
    lcd.print(' ');
    lcd.setCursor(x0 + w0, y0);
    lcd.print(' ');
  }
}

//////////////////////////// class Time View ///////////////////////////

TimeView::TimeView(byte x, byte y):View(x, y, 2) {
  mode = VIEW_MODE_TIME;
}

void TimeView::draw(byte xgroup) {
  // Draws number 00-99 at position [x0,y0] in mode 
  // if number < 0 - no action
 
 
  if (!isVisible()) return;
  
  
  if (time < 0) return; //no draw negative numbers

  lcd.setCursor(x0, y0);
    
  char character;
  character = time / 10 + '0';
  lcd.write(character);
  
  character = time % 10 + '0';
  lcd.write(character);
}

////////////////// class ButtonView /////////////////////////
//

ButtonView::ButtonView(byte x, byte y, byte w, String xtext) : View(x, y, w), text(xtext) {
  mode = VIEW_MODE_BUTTON;
};

void ButtonView::draw(byte xgroup) {
  //drawSelector(isSelected());
  if (!isVisible()) return;
  if (isClear()) {
    clear();
    return;
  }
  lcd.setCursor(x0, y0);
  lcd.print(text);
}



////////////////  LABEL VIEW  /////////////////////////////////////

LabelView::LabelView(byte x, byte y, byte w, String xtext) : View(x, y, w), text(xtext) {
  mode = VIEW_MODE_LABEL;
};

void LabelView::draw(byte xgroup) {
  
  if (!isVisible()) return;
  if (isClear()) {
    clear();
    return;
  }
  lcd.setCursor(x0, y0);
  lcd.print(text);
}

/*
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
*/