CC = g++
#CFLAGS = -Wall -g
CFLAGS = -Wall -O3
 
swdloader: main.o swdloader.o gpiopin.o
	$(CC) $(CFLAGS) -o swdloader main.o swdloader.o gpiopin.o
 
# The main.o target can be written more simply
 
main.o: main.cpp swdloader.h
	$(CC) $(CFLAGS) -c main.cpp
 
swdloader.o: swdloader.cpp swdloader.h gpiopin.h
	$(CC) $(CFLAGS) -c swdloader.cpp
 
gpiopin.o: gpiopin.cpp gpiopin.h
	$(CC) $(CFLAGS) -c gpiopin.cpp

clean:
	rm -rf *.o swdloader
