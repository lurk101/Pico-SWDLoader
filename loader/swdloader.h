//
// swdloader.h
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
#ifndef _pico_swdloader_h
#define _pico_swdloader_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "gpiopin.h"

struct CSWDLoader {
    unsigned m_bResetAvailable;
    unsigned m_nDelayNanos;
    struct CGPIOPin m_ResetPin;
    struct CGPIOPin m_ClockPin;
    struct CGPIOPin m_DataPin;
};

/// \param nClockPin GPIO pin to which SWCLK is connected
/// \param nDataPin GPIO pin to which SWDIO is connected
/// \param nResetPin Optional GPIO pin to which RESET (RUN) is connected
/// (active LOW) \param nClockRateKHz Requested interface clock rate in KHz
/// \note GPIO pin numbers are SoC number, not header positions.
/// \note The actual clock rate may be smaller than the requested.
int SWDInitialise(struct CSWDLoader* loader, unsigned nClockPin,
                  unsigned nDataPin, unsigned nResetPin,
                  unsigned nClockRateKHz);

void SWDDeInitialise(struct CSWDLoader* loader);

/// \brief Halt the RP2040, load a program image and start it
/// \param pProgram Pointer to program image in memory
/// \param nProgSize Size of the program image (must be a multiple of 4)
/// \param nAddress Load and start address of the program image
int SWDLoad(struct CSWDLoader* loader, const void* pProgram, size_t nProgSize,
            uint32_t nAddress);

/// \brief Halt the RP2040
/// \return Operation successful?
int SWDHalt(struct CSWDLoader* loader);

/// \brief Load a chunk of a program image (or entire program)
/// \param pChunk Pointer to the chunk in memory
/// \param nChunkSize Size of the chunk (must be a multiple of 4)
/// \param nAddress Load address of the chunk
/// \return Operation successful?
int SWDLoadChunk(struct CSWDLoader* loader, const void* pChunk,
                 size_t nChunkSize, uint32_t nAddress);

/// \brief Start program image
/// \param nAddress Start address of the program image
/// \return Operation successful?
int SWDStart(struct CSWDLoader* loader, uint32_t nAddress);

#ifdef __cplusplus
}
#endif

#endif
