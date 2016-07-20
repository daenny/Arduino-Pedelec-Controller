/*
ILI9341 LCD Display controller for Arduino_Pedelec_Controller

Copyright (C) 2016
Andreas Butti, andreas.b242 at gmail dot com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "DisplayController.h"

#include "MainView.h"
#include "MenuView.h"
#include "MainViewEdit.h"


#include "Components.h"
#include "BaseView.h"

/**
 * Control the whole display Navigation and output
 */

// Model with all data
DataModel model;

#define KEY A0

// If you change it, also change the interrupt initialisation!
#define KNOB0 A1
#define KNOB1 A2

// Setup a RoraryEncoder for pins A2 and A3:
//RotaryEncoder encoder(KNOB0, KNOB1);

//! Customizeable components on the main screen
Components components;

//! Main view with speed etc.
MainView mainView(&components);

//! Edit custmizeable part of the main view
MainViewEdit mainViewEdit(&components);

//! Menu view to show a menu
MenuView menuView;

//! Current active view
BaseView *currentView;

//! Key pressed flag
//volatile bool g_keyPressed = false;
ILI9341_t3 lcd = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST);

boolean repaint = true;

//! Call once on startup
void displayControllerSetup() {
    lcd.begin();
    repaint = true;
    currentView = &mainView;
    currentView->activate();

    // Button
    //pinMode(KEY, INPUT);
    //pinMode(KNOB0, INPUT);
    //pinMode(KNOB1, INPUT);

    // enable pullup
    //digitalWrite(KEY, HIGH);
    //digitalWrite(KNOB0, HIGH);
    //digitalWrite(KNOB1, HIGH);

    // You may have to modify the next 2 lines if using other pins than A1 and A2
    // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
    // PCICR |= (1 << PCIE1);

    // This enables the interrupt for pin 1 and 2 of Port C.
    //PCMSK1 |= (1 << PCINT9) | (1 << PCINT10);

    // Run timer2 interrupt every 15 ms
//  TCCR2A = 0;
    // TCCR2B = 1 << CS22 | 1 << CS21 | 1 << CS20;

    // Timer2 Overflow Interrupt Enable
    // TIMSK2 |= 1 << TOIE2;
}

//! Timer 2 interrupt, for button debouncing
/*SIGNAL(TIMER2_OVF_vect) {
  static bool lastState = 1;
  bool currentState = digitalRead(KEY);

  if (currentState == lastState) {
    return;
  }

  if (currentState == 0) {
    g_keyPressed = true;
  }

  lastState = currentState;
}

// Pin Change Interrupt for Rotation encoder, A1 and A2
ISR(PCINT1_vect) {

  // Handle rotary interrupts
  encoder.tick();
}
*/
void updatePosition(int8_t diff) {
    currentView->movePosition(diff);
}

void updateDisplay() {
    if (repaint) {
        currentView->updateDisplay();
        repaint = false;
    }
    //always draw Diagramm if active TODO add data listener?
    if (g_components[COMP_ID_DIAG]->is_active())
        g_components[COMP_ID_DIAG]->draw();
}

int keyPressed() {
    ViewResult result = currentView->keyPressed();
    Serial.println("KEY Result: ");
    Serial.print(result.result);
    Serial.print(result.value);
    Serial.println();
    int response = 0;

    if (result.result == VIEW_RESULT_MENU) {
        currentView->deactivate();
        menuView.setRootMenuId(result.value);
        currentView = &menuView;
        currentView->activate();
    } else if (result.result == VIEW_RESULT_BACK) {
        currentView->deactivate();
        currentView = &mainView;
        repaint = true;
        currentView->activate();
    } else if (result.result == VIEW_RESULT_SELECTED) {
        currentView->deactivate();

        if (MENU_ID_VIEW_EDIT == result.value) {
            currentView = &mainViewEdit;
        } else if (MENU_ID_COMPONENT_REMOVE == result.value) {
            mainViewEdit.removeSelected();
            currentView = &mainViewEdit;
        } else {
            currentView = &mainView;
        }

        currentView->activate();
    } else if (result.result == VIEW_RESULT_CHECKBOX_CHECKED) {



        if (result.value == MENU_ID_LIGHT_CB) {
            model.showIcon(ICON_ID_LIGHT);
            response = DISPLAY_ACTION_TOGGLE_LIGHT_ON;

        }
        else if (result.value == MENU_ID_BLUETOOTH_CB) {
            model.showIcon(ICON_ID_BLUETOOTH);
            response = DISPLAY_ACTION_TOGGLE_BLUETOOTH_ON;
        }

        else if (result.value == MENU_ID_PROFIL_CB) {
            model.showIcon(ICON_ID_PROFILE);
            response = DISPLAY_ACTION_ACTIVE_PROFILE_1;
        }

        //! Checkbox toggled
    } else if (result.result == VIEW_RESULT_CHECKBOX_UNCHECKED) {
        if (result.value == MENU_ID_LIGHT_CB) {
            model.clearIcon(ICON_ID_LIGHT);
            response = DISPLAY_ACTION_TOGGLE_LIGHT_OFF;
        }
        else if (result.value == MENU_ID_BLUETOOTH_CB) {
            model.clearIcon(ICON_ID_BLUETOOTH);
            response = DISPLAY_ACTION_TOGGLE_BLUETOOTH_OFF;
        }
        else if (result.value == MENU_ID_PROFIL_CB) {
            model.clearIcon(ICON_ID_PROFILE);
            response = DISPLAY_ACTION_ACTIVE_PROFILE_2;
        }

    }
    return response;
}

/*
//! Execute 1 byte command
void displayControlerCommand1(uint8_t cmd, uint8_t value) {
  switch (cmd) {
    case DISP_CMD_STATES:
      if(value & DISP_BIT_STATE_BLUETOOTH) {
        model.showIcon(ICON_ID_BLUETOOTH);
      } else {
        model.clearIcon(ICON_ID_BLUETOOTH);
      }

      if(value & DISP_BIT_STATE_BRAKE) {
        model.showIcon(ICON_ID_BRAKE);
      } else {
        model.clearIcon(ICON_ID_BRAKE);
      }

      if(value & DISP_BIT_STATE_LIGHT) {
        model.showIcon(ICON_ID_LIGHT);
      } else {
        model.clearIcon(ICON_ID_LIGHT);
      }
//  ICON_ID_HEART     = (1 << 3)


      break;
  }
}

void updateValues(uint16_t bat_perc, float_t speed, )

//! Execute 2 byte command
void displayControlerCommand2(uint8_t cmd, uint16_t value) {
  switch (cmd) {
    case DISP_CMD_BATTERY:
      model.setValue(VALUE_ID_BATTERY_VOLTAGE_CURRENT, value);
      break;
    case DISP_CMD_BATTERY_MAX:
      model.setValue(VALUE_ID_BATTERY_VOLTAGE_MAX, value);
      break;
    case DISP_CMD_BATTERY_MIN:
      model.setValue(VALUE_ID_BATTERY_VOLTAGE_MIN, value);
      break;
    case DISP_CMD_SPEED:
      model.setValue(VALUE_ID_SPEED, value);
      break;
    case DISP_CMD_WATTAGE:
      mainView.setWattage(value);
      break;
  }
}*/
