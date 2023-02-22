//
// gpiopin.c
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
    assert(nPin < GPIO_PINS);
    pin->m_nPin = nPin;
}

void SetModePin(struct CGPIOPin* pin, enum TGPIOMode Mode, int bInitPin) {
    assert(Mode < GPIOModeUnknown);
    pin->m_Mode = Mode;
    switch (Mode) {
    case GPIOModeInput:
        gpioSetMode(pin->m_nPin, PI_INPUT);
        break;
    case GPIOModeOutput:
        gpioSetMode(pin->m_nPin, PI_OUTPUT);
        if (bInitPin)
            WritePin(pin, LOW);
        /* fall-through */
    default:
        break;
    }
}

void WritePin(struct CGPIOPin* pin, unsigned nValue) {
    assert(pin->m_nPin < GPIO_PINS);
    assert(pin->m_Mode < GPIOModeUnknown);
    assert(nValue == LOW || nValue == HIGH);
    gpioWrite(pin->m_nPin, nValue);
}

unsigned ReadPin(struct CGPIOPin* pin) {
    assert(pin->m_nPin < GPIO_PINS);
    assert(pin->m_Mode == GPIOModeInput);
    return gpioRead(pin->m_nPin);
}
