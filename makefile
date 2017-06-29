CC=g++
CFLAGS=-std=gnu++11 -Wall -Wfatal-errors -O2 -c
LFLAGS=-std=gnu++11 -Wall -Wfatal-errors -O2
EXEC=a.out

SOURCES=$(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf src/*.o
