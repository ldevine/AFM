CC = g++
INC_PATH = -I.
CFLAGS = -std=c++11 -O2 -march=native -mtune=native -w $(INC_PATH)
LIBS =  -lpthread 
LDFLAGS = $(LIBS)

all: afmfind



afmfind: ./AFMFind/AFMFind.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

	
clean: 
	rm -f *.o
cleanest: clean

	

	
	
	
