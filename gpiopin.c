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

#include "gpiopin.h"

#define CHIPNAME "gpiochip0"
#define CONSUMER "SWD"

void InitPin(struct CGPIOPin* pin, unsigned nPin, enum TGPIOMode Mode) {
    pin->m_nPin = GPIO_PINS;
    pin->m_Mode = GPIOModeUnknown;
    AssignPin(pin, nPin);
    SetModePin(pin, Mode, 1);
}

void DeInitPin(struct CGPIOPin* pin) {
    SetModePin(pin, GPIOModeInputPullNone, 1);
#if defined(USE_LIBGPIOD)
    gpiod_line_release(pin->m_Line);
    gpiod_chip_close(pin->m_Chip);
#endif
}

void AssignPin(struct CGPIOPin* pin, unsigned nPin) {
    assert(nPin < GPIO_PINS);
    pin->m_nPin = nPin;
#if defined(USE_LIBGPIOD)
    pin->m_Chip = gpiod_chip_open_by_name(CHIPNAME);
    assert(pin->m_Chip);
    pin->m_Line = gpiod_chip_get_line(pin->m_Chip, nPin);
    assert(pin->m_Line);
#endif
}

void SetModePin(struct CGPIOPin* pin, enum TGPIOMode Mode, int bInitPin) {
    assert(Mode < GPIOModeUnknown);
    if (Mode == pin->m_Mode && !bInitPin)
        return;
    // on the fly direction change not supported!!!
#if defined(USE_LIBGPIOD)
    gpiod_line_release(pin->m_Line);
    pin->m_Mode = Mode;
    int r;
    switch (Mode) {
    case GPIOModeInputPullUp:
    case GPIOModeInputPullNone:
        r = gpiod_line_request_input(pin->m_Line, CONSUMER);
        assert(r >= 0);
        break;
    case GPIOModeOutput:
        if (bInitPin)
            pin->m_nLastWrite = LOW;
        r = gpiod_line_request_output(pin->m_Line, CONSUMER, pin->m_nLastWrite);
        assert(r >= 0);
        /* fall-through */
    default:
        break;
    }
#endif
#if defined(USE_LIBPIGPIO)
    pin->m_Mode = Mode;
    int r;
    switch (Mode) {
    case GPIOModeInputPullUp:
    case GPIOModeInputPullNone:
        if (bInitPin)
            switch (Mode) {
            case GPIOModeInputPullUp:
                r = gpioSetPullUpDown(pin->m_nPin, PI_PUD_UP);
                assert(r >= 0);
                break;
            case GPIOModeInputPullNone:
                r = gpioSetPullUpDown(pin->m_nPin, PI_PUD_OFF);
                assert(r >= 0);
                /* fall-through */
            default:
                break;
            }
        r = gpioSetMode(pin->m_nPin, PI_INPUT);
        assert(r >= 0);
        break;
    case GPIOModeOutput:
        r = gpioSetMode(pin->m_nPin, PI_OUTPUT);
        assert(r >= 0);
        if (bInitPin) {
            r = gpioSetPullUpDown(pin->m_nPin, PI_PUD_OFF);
            assert(r >= 0);
            WritePin(pin, LOW);
        }
        /* fall-through */
    default:
        break;
    }
#endif
}

void WritePin(struct CGPIOPin* pin, unsigned nValue) {
    assert(pin->m_nPin < GPIO_PINS);
    assert(pin->m_Mode < GPIOModeUnknown);
    assert(nValue == LOW || nValue == HIGH);
#if defined(USE_LIBGPIOD)
    pin->m_nLastWrite = nValue;
    if (pin->m_Mode == GPIOModeOutput) {
        int r = gpiod_line_set_value(pin->m_Line, nValue);
        assert(r >= 0);
    }
#endif
#if defined(USE_LIBPIGPIO)
    int r = gpioWrite(pin->m_nPin, nValue);
    assert(r >= 0);
#endif
}

unsigned ReadPin(struct CGPIOPin* pin) {
    assert(pin->m_nPin < GPIO_PINS);
    assert(pin->m_Mode == GPIOModeInputPullUp ||
           pin->m_Mode == GPIOModeInputPullNone);
    int r;
#if defined(USE_LIBGPIOD)
    r = gpiod_line_get_value(pin->m_Line);
#endif
#if defined(USE_LIBPIGPIO)
    r = gpioRead(pin->m_nPin);
#endif
    assert(r >= 0);
    return r;
}
