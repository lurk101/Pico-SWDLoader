#CFLAGS = -Wall -g
CFLAGS = -Wall -O3 -flto
LDFLAGS =
LDLIBS = -lgpiod
 
swdloader: main.o swdloader.o gpiopin.o
	$(CC) $(LDFLAGS) -o swdloader main.o swdloader.o gpiopin.o $(LDLIBS)
 
main.o: main.c swdloader.h
	$(CC) $(CFLAGS) -c main.c
 
swdloader.o: swdloader.c swdloader.h gpiopin.h
	$(CC) $(CFLAGS) -c swdloader.c
 
gpiopin.o: gpiopin.c gpiopin.h
	$(CC) $(CFLAGS) -c gpiopin.c

clean:
	rm -rf *.o swdloader
