#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pigpio.h>

#include "swdloader.h"

#define SWCLK_PIN 25
#define SWDIO_PIN 24
#define RESET_PIN 0 // 0 for none

#define RP2040_RAM_BASE 0x20000000U

int main(int ac, char* av[]) {
    int swdio_gpio = SWDIO_PIN, swclk_gpio = SWCLK_PIN;
    char* f_name;
    if (ac < 2) {
        fprintf(
            stderr,
            "Usage: swdloader image_file_name [ swdio_gpio [ swclk_gpio ]]\n");
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
        exit(-1);
    }
    if ((f_size & 3) != 0) {
        fprintf(stderr, "Image size must be multiple of 4\n");
        exit(-1);
    }
    lseek(fd, 0, SEEK_SET);
    printf("File size %lu bytes\n", f_size);
    char* image = (char*)malloc(f_size);
    if (image == NULL) {
        fprintf(stderr, "Not enough memory\n");
        exit(-1);
    }
    if (read(fd, image, f_size) != f_size) {
        fprintf(stderr, "Error reading %s\n", f_name);
        exit(-1);
    }
    close(fd);
    gpioInitialise();
    struct CSWDLoader loader;
    if (!SWDInitialize(&loader, swclk_gpio, swdio_gpio, RESET_PIN, 1000)) {
        fprintf(stderr, "Firmware init failed\n");
        exit(-1);
    }
    if (!SWDLoad(&loader, image, f_size, RP2040_RAM_BASE)) {
        fprintf(stderr, "Firmware load failed\n");
        exit(-1);
    }
    return 0;
}
