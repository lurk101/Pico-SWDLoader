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
    int swdio_gpio = SWDIO_PIN, swclk_gpio = SWCLK_PIN, rc = -1;
    char* f_name;
    if (ac < 2) {
        fprintf(
            stderr,
            "Usage: swdloader image_file_name [ swdio_gpio [ swclk_gpio ]]\n"
            "  Defaults: swdio = GPIO%d, swclk = GPIO%d\n",
            SWDIO_PIN, SWCLK_PIN);
        exit(-1);
    }
    if (geteuid() != 0) {
        fprintf(stderr, "swdloader needs root\n");
        exit(-1);
    }
    f_name = av[1];
    if (ac > 2)
        swdio_gpio = atoi(av[2]);
    if (ac > 3)
        swclk_gpio = atoi(av[3]);
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
    if (!SWDInitialise(&loader, swclk_gpio, swdio_gpio, RESET_PIN, 1000)) {
        fprintf(stderr, "Firmware init failed\n");
        goto exit_swd;
    }
    swdInitialized = 1;
    printf("SWD dio = GPIO%d, clk = GPIO%d\n", swdio_gpio, swclk_gpio);
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
