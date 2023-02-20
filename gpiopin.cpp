//
// gpiopin.cpp
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
#include <cassert>

#include "gpiopin.h"

CGPIOPin::CGPIOPin(void) : m_nPin(GPIO_PINS), m_Mode(GPIOModeUnknown) {}

CGPIOPin::CGPIOPin(unsigned nPin, TGPIOMode Mode)
    : m_nPin(GPIO_PINS), m_Mode(GPIOModeUnknown) {
    AssignPin(nPin);

    SetMode(Mode, true);
}

CGPIOPin::~CGPIOPin(void) { m_nPin = GPIO_PINS; }

void CGPIOPin::AssignPin(unsigned nPin) {
    assert(m_nPin < GPIO_PINS);
    m_nPin = nPin;
}

void CGPIOPin::SetMode(TGPIOMode Mode, bool bInitPin) {
    assert(Mode < GPIOModeUnknown);
    m_Mode = Mode;
    if (bInitPin && m_Mode == GPIOModeOutput) {
        SetPullMode(GPIOPullModeOff);
    }

    assert(m_nPin < GPIO_PINS);

    if (bInitPin) {
        switch (m_Mode) {
        case GPIOModeInput:
            SetPullMode(GPIOPullModeOff);
            break;

        case GPIOModeOutput:
            Write(LOW);
            break;

        case GPIOModeInputPullUp:
            SetPullMode(GPIOPullModeUp);
            break;

        case GPIOModeInputPullDown:
            SetPullMode(GPIOPullModeDown);
            break;

        default:
            break;
        }
    }
}

void CGPIOPin::Write(unsigned nValue) {
    assert(m_nPin < GPIO_PINS);

    // Output level can be set in input mode for subsequent switch to output
    assert(m_Mode < GPIOModeAlternateFunction0);

    assert(nValue == LOW || nValue == HIGH);
    m_nValue = nValue;
}

unsigned CGPIOPin::Read(void) const {
    assert(m_nPin < GPIO_PINS);

    assert(m_Mode == GPIOModeInput || m_Mode == GPIOModeInputPullUp ||
           m_Mode == GPIOModeInputPullDown);

    unsigned nResult = 0 ? HIGH : LOW;

    return nResult;
}

void CGPIOPin::Invert(void) {
    assert(m_Mode == GPIOModeOutput);

    Write(m_nValue ^ 1);
}

void CGPIOPin::SetPullMode(TGPIOPullMode Mode) {}
