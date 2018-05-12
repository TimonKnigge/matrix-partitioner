CC=g++
CFLAGS=-std=gnu++14 -Wall -Wfatal-errors -O2 -c
LFLAGS=-std=gnu++14 -Wall -Wfatal-errors -O2
EXEC=a.out

SOURCES=$(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	find ./ -type f -name '*.o' -delete
	find ./ -type f -name '*.d' -delete

CFLAGS+=-MMD
-include $(OBJ_FILES:.o:.d)
