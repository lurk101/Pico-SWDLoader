#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pigpio.h>

#include "swdloader.h"

#define SWCLK_PIN 25
#define SWDIO_PIN 24
#define RESET_PIN 0 // 0 for none

#define RP2040_RAM_BASE 0x20000000U

int main(int ac, char* av[]) {
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
    int fd = open(f_name, 0);
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
    printf("Image size %lu bytes\n", f_size);
    char* image = (char*)mmap(NULL, f_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (image == MAP_FAILED) {
        fprintf(stderr, "Not enough memory\n");
        goto exit_fd;
    }
    gpioInitialise();
    struct CSWDLoader loader;
    if (!SWDInitialise(&loader, swclk_gpio, swdio_gpio, RESET_PIN, 1000)) {
        fprintf(stderr, "Firmware init failed\n");
        goto exit_swd;
    }
    if (!SWDLoad(&loader, image, f_size, RP2040_RAM_BASE)) {
        fprintf(stderr, "Firmware load failed\n");
        goto exit_swd;
    }
    rc = 0;
exit_swd:
    SWDDeInitialise(&loader);
exit_fd:
    close(fd);
    return rc;
}
