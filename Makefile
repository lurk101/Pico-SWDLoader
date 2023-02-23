#CFLAGS = -Wall -g
CFLAGS = -Wall -O3 -flto
LDFLAGS =
LDLIBS = -lgpiod

CFLAGS += -DSWCLK_PIN=25
CFLAGS += -DSWDIO_PIN=24
CFLAGS += -DRESET_PIN=0 # 0 for none
#CFLAGS += -DRESET_PIN=23
CFLAGS += -DRAM_BASE=0x20000000U
 
swdloader: main.o swdloader.o gpiopin.o Makefile
	$(CC) $(LDFLAGS) -o swdloader main.o swdloader.o gpiopin.o $(LDLIBS)
 
main.o: main.c swdloader.h Makefile
	$(CC) $(CFLAGS) -c main.c
 
swdloader.o: swdloader.c swdloader.h gpiopin.h Makefile
	$(CC) $(CFLAGS) -c swdloader.c
 
gpiopin.o: gpiopin.c gpiopin.h Makefile
	$(CC) $(CFLAGS) -c gpiopin.c

clean:
	rm -rf *.o swdloader
