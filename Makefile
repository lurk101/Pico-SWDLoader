#CFLAGS = -Wall -g
CFLAGS = -Wall -O3 -flto
LDFLAGS =
LDLIBS = -lpigpio
 
swdloader: main.o swdloader.o gpiopin.o
	$(CXX) $(LDFLAGS) -o swdloader main.o swdloader.o gpiopin.o $(LDLIBS)
 
main.o: main.cpp swdloader.h
	$(CXX) $(CFLAGS) -c main.cpp
 
swdloader.o: swdloader.cpp swdloader.h gpiopin.h
	$(CXX) $(CFLAGS) -c swdloader.cpp
 
gpiopin.o: gpiopin.cpp gpiopin.h
	$(CXX) $(CFLAGS) -c gpiopin.cpp

clean:
	rm -rf *.o swdloader
