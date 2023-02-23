//
// gpiopin.h
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
#ifndef _circle_gpiopin_h
#define _circle_gpiopin_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#if defined(USE_LIBGPIOD)
#include <gpiod.h>
#elif defined(USE_LIBPIGPIO)
#include <pigpio.h>
#else
#error Must define USE_LIBGPIOD or USE_LIBPIGPIO
#endif

#define GPIO_PINS 54
#define LOW 0
#define HIGH 1

enum TGPIOMode {
    GPIOModeInputPullUp,
    GPIOModeInputPullNone,
    GPIOModeOutput,
    GPIOModeUnknown
};

struct CGPIOPin {
    unsigned m_nPin;
    enum TGPIOMode m_Mode;
#if defined(USE_LIBGPIOD)
    struct gpiod_chip* m_Chip;
    struct gpiod_line* m_Line;
    unsigned m_nLastWrite;
#endif
};

/// \param nPin Pin number, can be physical (Broadcom) number or
/// TGPIOVirtualPin \param Mode Pin mode to be set \param pManager Is only
/// required for using interrupts (IRQ)
void InitPin(struct CGPIOPin* pin, unsigned nPin, enum TGPIOMode Mode);
void DeInitPin(struct CGPIOPin* pin);

/// \param nPin Pin number, can be physical (Broadcom) number or
/// TGPIOVirtualPin \note To be used together with the default constructor
/// and SetMode()
void AssignPin(struct CGPIOPin* pin, unsigned nPin);

/// \param Mode Pin mode to be set
/// \param bInitPin Also init pullup/down mode and output level
void SetModePin(struct CGPIOPin* pin, enum TGPIOMode Mode, int bInitPin);

/// \param nValue Value to be written to the pin (LOW or HIGH)
void WritePin(struct CGPIOPin* pin, unsigned nValue);

/// \return Value read from pin (LOW or HIGH)
unsigned ReadPin(struct CGPIOPin* pin);

#ifdef __cplusplus
}
#endif

#endif
