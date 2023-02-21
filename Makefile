#CFLAGS = -Wall -g
CFLAGS = -Wall -O3 -flto
LDFLAGS =
LDLIBS = -lpigpio
 
swdloader: main.o swdloader.o gpiopin.o
	$(CXX) $(LDFLAGS) -o swdloader main.o swdloader.o gpiopin.o $(LDLIBS)
 
main.o: main.cpp swdloader.h
	$(CXX) $(CFLAGS) -c main.cpp
 
swdloader.o: swdloader.c swdloader.h gpiopin.h
	$(CC) $(CFLAGS) -c swdloader.c
 
gpiopin.o: gpiopin.c gpiopin.h
	$(CC) $(CFLAGS) -c gpiopin.c

clean:
	rm -rf *.o swdloader
