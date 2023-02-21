//
// gpiopin.c
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2020  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <assert.h>
#include <pigpio.h>

#include "gpiopin.h"

void InitPin(struct CGPIOPin* pin, unsigned nPin, enum TGPIOMode Mode) {
    pin->m_nPin = GPIO_PINS;
    pin->m_Mode = GPIOModeUnknown;
    AssignPin(pin, nPin);
    SetModePin(pin, Mode, 1);
}

void AssignPin(struct CGPIOPin* pin, unsigned nPin) {
    pin->m_nPin = nPin;
    assert(pin->m_nPin < GPIO_PINS);
}

//    GPIOModeInput,
//    GPIOModeOutput,
//    GPIOModeInputPullUp,
//    GPIOModeInputPullDown,

void SetModePin(struct CGPIOPin* pin, enum TGPIOMode Mode, int bInitPin) {
    assert(Mode < GPIOModeUnknown);
    pin->m_Mode = Mode;

    if (bInitPin && pin->m_Mode == GPIOModeOutput)
        SetPullModePin(pin, GPIOPullModeOff);

    switch (pin->m_Mode) {
    case GPIOModeInput:
    case GPIOModeInputPullUp:
    case GPIOModeInputPullDown:
        gpioSetMode(pin->m_nPin, PI_INPUT);
        break;
    case GPIOModeOutput:
        gpioSetMode(pin->m_nPin, PI_OUTPUT);
    default:
        break;
    }

    if (bInitPin)
        switch (pin->m_Mode) {
        case GPIOModeInput:
            SetPullModePin(pin, GPIOPullModeOff);
            break;

        case GPIOModeOutput:
            WritePin(pin, LOW);
            break;

        case GPIOModeInputPullUp:
            SetPullModePin(pin, GPIOPullModeUp);
            break;

        case GPIOModeInputPullDown:
            SetPullModePin(pin, GPIOPullModeDown);
            break;

        default:
            break;
        }
}

void WritePin(struct CGPIOPin* pin, unsigned nValue) {
    assert(pin->m_nPin < GPIO_PINS);
    assert(pin->m_Mode < GPIOModeUnknown);
    assert(nValue == LOW || nValue == HIGH);
    gpioWrite(pin->m_nPin, nValue == LOW ? 0 : 1);
}

unsigned ReadPin(struct CGPIOPin* pin) {
    assert(pin->m_nPin < GPIO_PINS);
    assert(pin->m_Mode == GPIOModeInput || pin->m_Mode == GPIOModeInputPullUp ||
           pin->m_Mode == GPIOModeInputPullDown);
    return gpioRead(pin->m_nPin) ? HIGH : LOW;
}

void SetPullModePin(struct CGPIOPin* pin, enum TGPIOPullMode Mode) {
    switch (Mode) {
    case GPIOPullModeOff:
        gpioSetPullUpDown(pin->m_nPin, PI_PUD_OFF);
        break;

    case GPIOPullModeUp:
        gpioSetPullUpDown(pin->m_nPin, PI_PUD_UP);
        break;

    case GPIOPullModeDown:
        gpioSetPullUpDown(pin->m_nPin, PI_PUD_DOWN);
        break;

    default:
        break;
    }
}
