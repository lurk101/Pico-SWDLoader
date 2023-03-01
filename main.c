//
// main.c
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
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "swdloader.h"

#define RAM_BASE 0x20000000u
#define APROXIMATE_SWD_CLK_KHZ 500

static int fd = -1;
static int swdInitialized = 0;
static struct CSWDLoader loader;

static void INThandler(int sig) {
    signal(sig, SIG_IGN);
    fprintf(stderr, "\nInterrupted!\n");
    if (swdInitialized)
        SWDDeInitialise(&loader);
    if (fd >= 0)
        close(fd);
    exit(-1);
}

int main(int ac, char* av[]) {
    signal(SIGINT, INThandler);
    int swdio_gpio = SWDIO_GPIO, swclk_gpio = SWCLK_GPIO,
        swrst_gpio = SWRST_GPIO, swfreq = APROXIMATE_SWD_CLK_KHZ, rc = -1;
    char* f_name;
    if (ac < 2) {
    help:
        fprintf(stderr,
                "Usage: swdloader [-d n] [-c n] [-r n] [-f n] image_file_name\n"
                " -d n  SWD Data IO GPIO # (default = %d)\n"
                " -c n  SWD Clock GPIO # (default = %d)\n"
                " -r n  SWD Reset GPIO # (default = %d)\n"
                " -f n  SWD Clock Frequency in KHz (default = %d)\n",
                swdio_gpio, swclk_gpio, swrst_gpio, swfreq);
        exit(-1);
    }
    int opt;

    while ((opt = getopt(ac, av, "d:c:r:f:")) != -1) {
        switch (opt) {
        case 'd':
            swdio_gpio = atoi(optarg);
            break;
        case 'c':
            swclk_gpio = atoi(optarg);
            break;
        case 'r':
            swrst_gpio = atoi(optarg);
            break;
        case 'f':
            swfreq = atoi(optarg);
            break;
        default:
            goto help;
        }
    }
    if (optind >= ac) {
        fprintf(stderr, "image file name is required\n");
        goto help;
    }
    f_name = av[optind];

    if (geteuid() != 0) {
        fprintf(stderr, "swdloader needs root\n");
        exit(-1);
    }

    fd = open(f_name, 0);
    if (fd < 0) {
        fprintf(stderr, "Can't open %s\n", f_name);
        exit(-1);
    }
    off_t f_size = lseek(fd, 0, SEEK_END);
    if (f_size < 0) {
        fprintf(stderr, "Can't get size of %s\n", f_name);
        goto exit_fd;
    }
    if ((f_size & 3) != 0) {
        fprintf(stderr, "Image size must be multiple of 4\n");
        goto exit_fd;
    }
    lseek(fd, 0, SEEK_SET);
    printf("Image size %lu bytes (0x%08x-0x%08x)\n", f_size, RAM_BASE,
           RAM_BASE + (unsigned int)f_size);
    char* image = (char*)mmap(NULL, f_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (image == MAP_FAILED) {
        fprintf(stderr, "Not enough memory\n");
        goto exit_fd;
    }
#if defined(USE_LIBPIGPIO)
    int cfg = gpioCfgGetInternals();
    cfg |= PI_CFG_NOSIGHANDLER; // (1<<10)
    gpioCfgSetInternals(cfg);
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio initialization failed!\n");
        goto exit_fd;
    }
#endif
    printf("SWD dio = GPIO%d, clk = GPIO%d", swdio_gpio, swclk_gpio);
    if (swrst_gpio)
        printf(", rst = GPIO%d", swrst_gpio);
    printf("\n");
    if (!SWDInitialise(&loader, swclk_gpio, swdio_gpio, swrst_gpio, swfreq)) {
        fprintf(stderr, "Firmware init failed\n");
        goto exit_swd;
    }
    swdInitialized = 1;
    if (!SWDLoad(&loader, image, f_size, RAM_BASE)) {
        fprintf(stderr, "Firmware load failed\n");
        goto exit_swd;
    }
    rc = 0;
exit_swd:
    SWDDeInitialise(&loader);
#if defined(USE_LIBPIGPIO)
    gpioTerminate();
#endif
exit_fd:
    close(fd);
    return rc;
}
