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

Running using default GPIOs 24 & 25
NOTE: Must run with root priviledge
```
sudo ./swdloader uart.bin
```

Help
```
./swdloader
```
