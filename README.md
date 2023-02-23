This Linux program will load a Raspberry Pico program binary built with -DPICO_NO_FLASH=1 directly to Pico RAM and start the program
at 0x20000000. Generic SWD pin bit banging is achieved using libgpiod support.

Building

For Raspberry Pi3 or Pi4

swdio GPIO24. swclk GPIO25
```
sudo apt install libpigpio-dev ## only if not already installed
git clone https://github.com/lurk101/Pico-SWDLoader.git
cd Pico-SWDLoader
make -f MAkefile.pigpio
```

For generic Linux SBC (works on Pi)

swdio GPIO24. swclk GPIO25
```
sudo apt install libgpiod-dev ## only if not already installed
git clone https://github.com/lurk101/Pico-SWDLoader.git
cd Pico-SWDLoader
make -f MAkefile.gpiod
```

For generic Linux on rock-5b

swdio GPIO44. swclk GPIO45, swrst GPIO149
```
sudo apt install libgpiod-dev ## only if not already installed
git clone https://github.com/lurk101/Pico-SWDLoader.git
cd Pico-SWDLoader
make -f MAkefile.rock-5b
```

Running using default GPIOs

NOTE: Must run with root priviledge
```
sudo ./swdloader uart.bin
```

Help
```
./swdloader
```

NOTE: Using the generic (libgpiod) version

Each SoC has one or more gpio blocks each supporting a given number of lines. For example, the Pi4 has 2 gpio blocks,
the 1st with 58 lines and the 2nd with 8 lines. On the other hand the rock-5b has 5 gpio block with 32 lines each and
a 6th gpio block with 3 lines. In both these SoC the logical gpio numbers are mapped directly onto these blocks so
it is possible to calculate the line and block indices. Not all SoC necessarilly work this way! You can get some
idea of the mapping using the gpioinfo command.
