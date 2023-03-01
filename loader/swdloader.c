//
// swdloader.c
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
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "swdloader.h"

// References:
//
// [1] ARM Debug Interface Architecture Specification ADIv5.0 to ADIv5.2, IHI
// 0031E [2] ARM v6-M Architecture Reference Manual, DDI 0419E
// Debug Port v2

#define TURN_CYCLES 1

#define BIT(x) (1 << (x))

#define XIP_CNTL 0x14000000
#define USB_CNTL 0x50110040

// SWD-DP Requests
#define WR_DP_ABORT 0x81
#define DP_ABORT_STKCMPCLR BIT(1)
#define DP_ABORT_STKERRCLR BIT(2)
#define DP_ABORT_WDERRCLR BIT(3)
#define DP_ABORT_ORUNERRCLR BIT(4)
#define RD_DP_CTRL_STAT 0x8D
#define WR_DP_CTRL_STAT 0xA9
#define DP_CTRL_STAT_ORUNDETECT BIT(0)
#define DP_CTRL_STAT_STICKYERR BIT(5)
#define DP_CTRL_STAT_CDBGPWRUPREQ BIT(28)
#define DP_CTRL_STAT_CDBGPWRUPACK BIT(29)
#define DP_CTRL_STAT_CSYSPWRUPREQ BIT(30)
#define DP_CTRL_STAT_CSYSPWRUPACK BIT(31)
#define RD_DP_DPIDR 0xA5
#define DP_DPIDR_SUPPORTED 0x0BC12477
#define RD_DP_RDBUFF 0xBD
#define WR_DP_SELECT 0xB1
#define DP_SELECT_DPBANKSEL__SHIFT 0
#define DP_SELECT_APBANKSEL__SHIFT 4
#define DP_SELECT_APSEL__SHIFT 24
#define DP_SELECT_DEFAULT 0 // DP bank 0, AP 0, AP bank 0
#define WR_DP_TARGETSEL 0x99
#define DP_TARGETSEL_CPUAPID_SUPPORTED 0x01002927
#define DP_TARGETSEL_TINSTANCE__SHIFT 28
#define DP_TARGETSEL_TINSTANCE_CORE0 0
#define DP_TARGETSEL_TINSTANCE_CORE1 1

// SW-DP response
#define DP_OK 0b001
#define DP_WAIT 0b010
#define DP_FAULT 0b100

// SWD MEM-AP Requests
#define WR_AP_CSW 0xA3
#define AP_CSW_SIZE__SHIFT 0
#define AP_CSW_SIZE_32BITS 2
#define AP_CSW_ADDR_INC__SHIFT 4
#define AP_CSW_SIZE_INCREMENT_SINGLE 1
#define AP_CSW_DEVICE_EN BIT(6)
#define AP_CSW_PROT__SHIFT 24
#define AP_CSW_PROT_DEFAULT 0x22
#define AP_CSW_DBG_SW_ENABLE BIT(31)
#define RD_AP_DRW 0x9F
#define WR_AP_DRW 0xBB
#define WR_AP_TAR 0x8B

// ARMv6-M Debug System Registers
#define DHCSR 0xE000EDF0
#define DHCSR_C_DEBUGEN BIT(0)
#define DHCSR_C_HALT BIT(1)
#define DHCSR_DBGKEY__SHIFT 16
#define DHCSR_DBGKEY_KEY 0xA05F
#define DCRSR 0xE000EDF4
#define DCRSR_REGSEL__SHIFT 0
#define DCRSR_REGSEL_R15 15 // PC register
#define DCRSR_REGW_N_R BIT(16)
#define DCRDR 0xE000EDF8

static void BeginTransaction(struct CSWDLoader* loader);
static void EndTransaction(struct CSWDLoader* loader);
static void Dormant2SWD(struct CSWDLoader* loader);
static void WriteIdle(struct CSWDLoader* loader);
static void LineReset(struct CSWDLoader* loader);
static void SelectTarget(struct CSWDLoader* loader, uint32_t nCPUAPID,
                         uint8_t uchInstanceID);
static int ReadData(struct CSWDLoader* loader, uint8_t nRequest,
                    uint32_t* pData);
static int PowerOn(struct CSWDLoader* loader);
static int WriteData(struct CSWDLoader* loader, uint8_t nRequest,
                     uint32_t nData);
static int WriteMem(struct CSWDLoader* loader, uint32_t nAddress,
                    uint32_t nData);
static int ReadMem(struct CSWDLoader* loader, uint32_t nAddress,
                   uint32_t* pData);
static void WriteBits(struct CSWDLoader* loader, uint32_t nBits,
                      unsigned nBitCount);
static uint32_t ReadBits(struct CSWDLoader* loader, unsigned nBitCount);
static void WriteClock(struct CSWDLoader* loader);

int SWDInitialise(struct CSWDLoader* loader, unsigned nClockPin,
                  unsigned nDataPin, unsigned nResetPin,
                  unsigned nClockRateKHz) {
    loader->m_bResetAvailable = nResetPin != 0;
    loader->m_nDelayNanos = 1000000U / nClockRateKHz / 2;
    InitPin(&loader->m_ClockPin, nClockPin, GPIOModeOutput);
    InitPin(&loader->m_DataPin, nDataPin, GPIOModeOutput);
    if (loader->m_bResetAvailable) {
        InitPin(&loader->m_ResetPin, nResetPin, GPIOModeOutput);
        const struct timespec ts = {0, 10000000};
        nanosleep(&ts, NULL);
        WritePin(&loader->m_ResetPin, LOW);
        nanosleep(&ts, NULL);
        WritePin(&loader->m_ResetPin, HIGH);
        nanosleep(&ts, NULL);
    }
    BeginTransaction(loader);
    Dormant2SWD(loader);
    WriteIdle(loader);
    LineReset(loader);
    // core 1 remains halted after reset
    SelectTarget(loader, DP_TARGETSEL_CPUAPID_SUPPORTED,
                 DP_TARGETSEL_TINSTANCE_CORE0);
    uint32_t nIDCode;
    if (!ReadData(loader, RD_DP_DPIDR, &nIDCode)) {
        fprintf(stderr, "Target does not respond\n");
        return 0;
    }
    if (nIDCode != DP_DPIDR_SUPPORTED) {
        EndTransaction(loader);
        fprintf(stderr, "Debug target not supported (ID code 0x%X)\n", nIDCode);
        return 0;
    }
    if (!PowerOn(loader)) {
        fprintf(stderr, "Target connect failed\n");
        return 0;
    }
    EndTransaction(loader);
    return 1;
}

void SWDDeInitialise(struct CSWDLoader* loader) {
    DeInitPin(&loader->m_DataPin);
    DeInitPin(&loader->m_ClockPin);
#if defined(USE_LIBPIGPIO)
    // Leave reset high
    if (loader->m_bResetAvailable)
        DeInitPin(&loader->m_ResetPin);
#endif
}

static long timediff(clock_t t1, clock_t t2) {
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}

int SWDLoad(struct CSWDLoader* loader, const void* pProgram, size_t nProgSize,
            uint32_t nAddress) {
    if (!SWDHalt(loader))
        return 0;
    clock_t nStart, nEnd;
    nStart = clock();
    printf("Disabling XIP and USB\n");
    BeginTransaction(loader);
    if (!WriteData(loader, WR_AP_TAR, XIP_CNTL)) {
        fprintf(stderr, "\nCannot write TAR (0x%X)\n", XIP_CNTL);
        return 0;
    }
    if (!WriteData(loader, WR_AP_DRW, 0)) {
        fprintf(stderr, "\nMemory write failed (0x%X)\n", XIP_CNTL);
        return 0;
    }
    EndTransaction(loader);
    BeginTransaction(loader);
    if (!WriteData(loader, WR_AP_TAR, USB_CNTL)) {
        fprintf(stderr, "\nCannot write TAR (0x%X)\n", USB_CNTL);
        return 0;
    }
    if (!WriteData(loader, WR_AP_DRW, 0)) {
        fprintf(stderr, "\nMemory write failed (0x%X)\n", USB_CNTL);
        return 0;
    }
    EndTransaction(loader);
    if (!SWDLoadChunk(loader, pProgram, nProgSize, nAddress))
        return 0;
    nEnd = clock();
    double diff_t = timediff(nStart, nEnd) / 1000.0;
    printf("\n%lu bytes loaded in %.2f seconds (%.1f KBytes/s)\n", nProgSize,
           diff_t, nProgSize / diff_t / 1024.0);
    return SWDStart(loader, nAddress);
}

int SWDHalt(struct CSWDLoader* loader) {
    BeginTransaction(loader);
    if (!WriteData(
            loader, WR_AP_CSW,
            (AP_CSW_SIZE_32BITS << AP_CSW_SIZE__SHIFT) |
                (AP_CSW_SIZE_INCREMENT_SINGLE << AP_CSW_ADDR_INC__SHIFT) |
                AP_CSW_DEVICE_EN | (AP_CSW_PROT_DEFAULT << AP_CSW_PROT__SHIFT) |
                AP_CSW_DBG_SW_ENABLE) ||
        !WriteMem(loader, DHCSR,
                  DHCSR_C_DEBUGEN | DHCSR_C_HALT |
                      (DHCSR_DBGKEY_KEY << DHCSR_DBGKEY__SHIFT))) {
        fprintf(stderr, "Target halt failed\n");
        return 0;
    }
    EndTransaction(loader);
    return 1;
}

int SWDLoadChunk(struct CSWDLoader* loader, const void* pChunk,
                 size_t nChunkSize, uint32_t nAddress) {
    assert(pChunk != 0);
    assert((nChunkSize & 3) == 0);
    int iChunkSize = nChunkSize;
    uint32_t* pChunk32 = (uint32_t*)pChunk;
    while (iChunkSize > 0) {
        printf("\rLoading @ 0x%08x", nAddress);
        fflush(stdout);
        BeginTransaction(loader);
        if (!WriteData(loader, WR_AP_TAR, nAddress)) {
            fprintf(stderr, "\nCannot write TAR (0x%X)\n", nAddress);
            return 0;
        }
        int nBlockSize = 1024;
        if (iChunkSize < nBlockSize)
            nBlockSize = iChunkSize;
        uint32_t* pChunk32_save = pChunk32;
        for (unsigned i = 0; i < nBlockSize; i += 4)
            if (!WriteData(loader, WR_AP_DRW, *pChunk32++)) {
                fprintf(stderr, "\nMemory write failed (0x%X)\n", nAddress);
                return 0;
            }
        EndTransaction(loader);
        BeginTransaction(loader);
        uint32_t nWordRead;
        if (!ReadMem(loader, nAddress, &nWordRead))
            fprintf(stderr, "\nMemory read failed (0x%X)\n", nAddress);
        EndTransaction(loader);
        if (nWordRead != *pChunk32_save) {
            fprintf(stderr, "\nData mismatch (0x%X != 0x%X)\n", nWordRead,
                    *pChunk32_save);
            return 0;
        }
        nAddress += nBlockSize;
        iChunkSize -= nBlockSize;
    }
    return 1;
}

int SWDStart(struct CSWDLoader* loader, uint32_t nAddress) {
    printf("Starting\n");
    BeginTransaction(loader);
    if (!WriteMem(loader, DCRDR, nAddress) ||
        !WriteMem(loader, DCRSR,
                  (DCRSR_REGSEL_R15 << DCRSR_REGSEL__SHIFT) | DCRSR_REGW_N_R) ||
        !WriteMem(loader, DHCSR,
                  DHCSR_C_DEBUGEN |
                      (DHCSR_DBGKEY_KEY << DHCSR_DBGKEY__SHIFT))) {
        fprintf(stderr, "Target start failed\n");
        return 0;
    }
    EndTransaction(loader);
    return 1;
}

int PowerOn(struct CSWDLoader* loader) {
    if (!WriteData(loader, WR_DP_ABORT,
                   DP_ABORT_STKCMPCLR | DP_ABORT_STKERRCLR | DP_ABORT_WDERRCLR |
                       DP_ABORT_ORUNERRCLR))
        return 0;
    if (!WriteData(loader, WR_DP_SELECT, DP_SELECT_DEFAULT))
        return 0;
    if (!WriteData(loader, WR_DP_CTRL_STAT,
                   DP_CTRL_STAT_ORUNDETECT | DP_CTRL_STAT_STICKYERR |
                       DP_CTRL_STAT_CDBGPWRUPREQ | DP_CTRL_STAT_CSYSPWRUPREQ))
        return 0;
    uint32_t nCtrlStat;
    if (!ReadData(loader, RD_DP_CTRL_STAT, &nCtrlStat))
        return 0;
    if (!(nCtrlStat & DP_CTRL_STAT_CDBGPWRUPACK) ||
        !(nCtrlStat & DP_CTRL_STAT_CSYSPWRUPACK)) {
        EndTransaction(loader);
        return 0;
    }
    return 1;
}

int WriteMem(struct CSWDLoader* loader, uint32_t nAddress, uint32_t nData) {
    return WriteData(loader, WR_AP_TAR, nAddress) &&
           WriteData(loader, WR_AP_DRW, nData);
}

int ReadMem(struct CSWDLoader* loader, uint32_t nAddress, uint32_t* pData) {
    return WriteData(loader, WR_AP_TAR, nAddress) &&
           ReadData(loader, RD_AP_DRW, pData) &&
           ReadData(loader, RD_DP_RDBUFF, pData);
}

int WriteData(struct CSWDLoader* loader, uint8_t nRequest, uint32_t nData) {
    WriteBits(loader, nRequest, 7);
    assert(nRequest & 0x80);
    ReadBits(loader, 1 + TURN_CYCLES); // park bit (not driven) and turn cycle
    uint32_t nResponse = ReadBits(loader, 3);
    ReadBits(loader, TURN_CYCLES);
    if (nResponse != DP_OK) {
        EndTransaction(loader);
        fprintf(stderr, "Cannot write (req 0x%02X, data 0x%X, resp %u)\n",
                (unsigned)nRequest, nData, nResponse);
        return 0;
    }
    WriteBits(loader, nData, 32);
    WriteBits(loader, __builtin_parity(nData), 1);
    return 1;
}

int ReadData(struct CSWDLoader* loader, uint8_t nRequest, uint32_t* pData) {
    WriteBits(loader, nRequest, 7);
    assert(nRequest & 0x80);
    ReadBits(loader, 1 + TURN_CYCLES); // park bit (not driven) and turn cycle
    uint32_t nResponse = ReadBits(loader, 3);
    if (nResponse != DP_OK) {
        ReadBits(loader, TURN_CYCLES);
        EndTransaction(loader);
        fprintf(stderr, "Cannot read (req 0x%02X, resp %u)\n",
                (unsigned)nRequest, nResponse);
        return 0;
    }
    uint32_t nData = ReadBits(loader, 32);
    uint32_t nParity = ReadBits(loader, 1);
    if (nParity != __builtin_parity(nData)) {
        ReadBits(loader, TURN_CYCLES);
        EndTransaction(loader);
        fprintf(stderr, "Parity error (req 0x%02X)\n", (unsigned)nRequest);
        return 0;
    }
    assert(pData != 0);
    *pData = nData;
    ReadBits(loader, TURN_CYCLES);
    return 1;
}

void SelectTarget(struct CSWDLoader* loader, uint32_t nCPUAPID,
                  uint8_t uchInstanceID) {
    uint32_t nWData =
        nCPUAPID | ((uint32_t)uchInstanceID << DP_TARGETSEL_TINSTANCE__SHIFT);
    WriteBits(loader, WR_DP_TARGETSEL, 7);
    ReadBits(loader, 1 + 5); // park bit and 5 bits not driven
    WriteBits(loader, nWData, 32);
    WriteBits(loader, __builtin_parity(nWData), 1);
}

void BeginTransaction(struct CSWDLoader* loader) { WriteIdle(loader); }

void EndTransaction(struct CSWDLoader* loader) { WriteIdle(loader); }

// Leaving dormant state and switch to SW-DP ([1] section B5.3.4)
void Dormant2SWD(struct CSWDLoader* loader) {
    WriteBits(loader, 0xFF, 8);         // 8 cycles high
    WriteBits(loader, 0x6209F392U, 32); // selection alert sequence
    WriteBits(loader, 0x86852D95U, 32);
    WriteBits(loader, 0xE3DDAFE9U, 32);
    WriteBits(loader, 0x19BC0EA2U, 32);
    WriteBits(loader, 0x0, 4);  // 4 cycles low
    WriteBits(loader, 0x1A, 8); // activation code
}

void LineReset(struct CSWDLoader* loader) {
    WriteBits(loader, 0xFFFFFFFFU, 32);
    WriteBits(loader, 0x00FFFFFU, 28);
}

void WriteIdle(struct CSWDLoader* loader) {
    WriteBits(loader, 0, 8);
    WritePin(&loader->m_ClockPin, LOW);
    SetModePin(&loader->m_DataPin, GPIOModeOutput, 0);
    WritePin(&loader->m_DataPin, LOW);
}

void WriteBits(struct CSWDLoader* loader, uint32_t nBits, unsigned nBitCount) {
    SetModePin(&loader->m_DataPin, GPIOModeOutput, 0);
    while (nBitCount--) {
        WritePin(&loader->m_DataPin, nBits & 1);
        WriteClock(loader);
        nBits >>= 1;
    }
}

uint32_t ReadBits(struct CSWDLoader* loader, unsigned nBitCount) {
    SetModePin(&loader->m_DataPin, GPIOModeInputPullUp, 0);
    uint32_t nBits = 0;
    unsigned nRemaining = nBitCount--;
    while (nRemaining--) {
        unsigned nLevel = ReadPin(&loader->m_DataPin);
        WriteClock(loader);
        nBits >>= 1;
        nBits |= nLevel << nBitCount;
    }
    return nBits;
}

static void delay_nanos(uint32_t n) {
    n *= 1;
    struct timespec start, now;
    clock_gettime(CLOCK_REALTIME, &start);
    uint32_t delta_ns;
    do {
        clock_gettime(CLOCK_REALTIME, &now);
        delta_ns = (now.tv_sec - start.tv_sec) * 1000000000 + now.tv_nsec -
                   start.tv_nsec;
    } while (delta_ns < n);
}

void WriteClock(struct CSWDLoader* loader) {
    WritePin(&loader->m_ClockPin, LOW);
    delay_nanos(loader->m_nDelayNanos);
    WritePin(&loader->m_ClockPin, HIGH);
    delay_nanos(loader->m_nDelayNanos);
}
