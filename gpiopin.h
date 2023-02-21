//
// gpiopin.h
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
#ifndef _circle_gpiopin_h
#define _circle_gpiopin_h

#include <stdint.h>

#define GPIO_PINS 54

enum TGPIOMode {
    GPIOModeInput,
    GPIOModeOutput,
    GPIOModeInputPullUp,
    GPIOModeInputPullDown,
    GPIOModeUnknown
};

enum TGPIOPullMode {
    GPIOPullModeOff,
    GPIOPullModeDown,
    GPIOPullModeUp,
    GPIOPullModeUnknown
};

class CGPIOPin /// Encapsulates a GPIO pin
{
  public:
    /// \brief Default constructor
    CGPIOPin(void);

    /// \param nPin Pin number, can be physical (Broadcom) number or
    /// TGPIOVirtualPin \param Mode Pin mode to be set \param pManager Is only
    /// required for using interrupts (IRQ)
    CGPIOPin(unsigned nPin, TGPIOMode Mode);
    virtual ~CGPIOPin(void);

    /// \param nPin Pin number, can be physical (Broadcom) number or
    /// TGPIOVirtualPin \note To be used together with the default constructor
    /// and SetMode()
    void AssignPin(unsigned nPin);

    /// \param Mode Pin mode to be set
    /// \param bInitPin Also init pullup/down mode and output level
    void SetMode(TGPIOMode Mode, bool bInitPin = true);

    /// \param Mode Pull mode to be set
    void SetPullMode(TGPIOPullMode Mode);

    /// \param nValue Value to be written to the pin (LOW or HIGH)
    void Write(unsigned nValue);
    /// \return Value read from pin (LOW or HIGH)
    unsigned Read(void) const;
#define LOW 0
#define HIGH 1

    /// \brief Write inverted value to pin
    void Invert(void);

  protected:
    unsigned m_nPin;
    TGPIOMode m_Mode;
};

#endif
