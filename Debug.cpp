// NAME: Debug.cpp
//
// DESC: Helper functions for debugging
//
// Copyright (c) 2018 by Andreas Trappmann. All rights reserved.
//
// This file is part of the PN5180 library for the Arduino environment.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
#include <Arduino.h>
#include "Debug.h"

#ifdef DEBUG

static const char hexChar[] = "0123456789ABCDEF";
static char hexBuffer[9];

char * formatHex(const uint8_t val) {
  hexBuffer[0] = hexChar[val >> 4];
  hexBuffer[1] = hexChar[val & 0x0f];
  hexBuffer[2] = '\0';
  return hexBuffer;
}

char * formatHex(const uint16_t val) {
  hexBuffer[0] = hexChar[(val >> 12) & 0x0f];
  hexBuffer[1] = hexChar[(val >> 8) & 0x0f];
  hexBuffer[2] = hexChar[(val >> 4) & 0x0f];
  hexBuffer[3] = hexChar[val & 0x0f];
  hexBuffer[4] = '\0';
  return hexBuffer;
}

char * formatHex(uint32_t val) {
  for (int i=7; i>=0; i--) {
    hexBuffer[i] = hexChar[val & 0x0f];
    val = val >> 4;
  }
  hexBuffer[8] = '\0';
  return hexBuffer;
}

void showIRQStatus(uint32_t irqStatus) {
    Serial.print(F("IRQ-Status 0x"));
    Serial.print(irqStatus, HEX);
    Serial.print(": [ ");
    if (irqStatus & (1<< 0)) Serial.print(F("RQ "));
    if (irqStatus & (1<< 1)) Serial.print(F("TX "));
    if (irqStatus & (1<< 2)) Serial.print(F("IDLE "));
    if (irqStatus & (1<< 3)) Serial.print(F("MODE_DETECTED "));
    if (irqStatus & (1<< 4)) Serial.print(F("CARD_ACTIVATED "));
    if (irqStatus & (1<< 5)) Serial.print(F("STATE_CHANGE "));
    if (irqStatus & (1<< 6)) Serial.print(F("RFOFF_DET "));
    if (irqStatus & (1<< 7)) Serial.print(F("RFON_DET "));
    if (irqStatus & (1<< 8)) Serial.print(F("TX_RFOFF "));
    if (irqStatus & (1<< 9)) Serial.print(F("TX_RFON "));
    if (irqStatus & (1<<10)) Serial.print(F("RF_ACTIVE_ERROR "));
    if (irqStatus & (1<<11)) Serial.print(F("TIMER0 "));
    if (irqStatus & (1<<12)) Serial.print(F("TIMER1 "));
    if (irqStatus & (1<<13)) Serial.print(F("TIMER2 "));
    if (irqStatus & (1<<14)) Serial.print(F("RX_SOF_DET "));
    if (irqStatus & (1<<15)) Serial.print(F("RX_SC_DET "));
    if (irqStatus & (1<<16)) Serial.print(F("TEMPSENS_ERROR "));
    if (irqStatus & (1<<17)) Serial.print(F("GENERAL_ERROR "));
    if (irqStatus & (1<<18)) Serial.print(F("HV_ERROR "));
    if (irqStatus & (1<<19)) Serial.print(F("LPCD "));
    Serial.println("]");
}

#endif
