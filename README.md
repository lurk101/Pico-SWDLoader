This Linux program will load a Raspberry Pico program binary built with -DPICO_NO_FLASH=1 directly to Pico RAM and start the program
at 0x20000000. Generic SWD pin bit banging is achieved using libgpiod support.

Building

```
sudo apt install libgpiod-dev ## only if not already installed
git clone https://github.com/lurk101/Pico-SWDLoader.git
cd Pico-SWDLoader
make
```

Running using default GPIOs 24 & 25
NOTE: Must run with root priviledge

```
sudo ./swdloader uart.img
```

Help

```
./swdloader
```
