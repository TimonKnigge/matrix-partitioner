CC=g++
CFLAGS=-std=gnu++11 -Wall -Wfatal-errors -O2 -c
LFLAGS=-std=gnu++11 -Wall -Wfatal-errors -O2
EXEC=a.out

RAWSOURCES=main.cpp matrix/matrix.cpp matrix/matrix-util.cpp io/input.cpp io/output.cpp
SOURCES=$(addprefix src/,$(RAWSOURCES))
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf src/*.o
	rm -rf $(EXEC)
