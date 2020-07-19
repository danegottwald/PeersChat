CC= g++
CFLAGS= -std=c++14 -Wall -Werror -pedantic -Wpedantic -O2
LFLAGS= $$(pkg-config --libs gtk+-3.0)
TARGET= main

all: $(TARGET)

$(TARGET): $(TARGET).o PC_Gui.o
	$(CC) $^ -o $(TARGET) $(LFLAGS)

$(TARGET).o: $(TARGET).cpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags gtk+-3.0) -c $<

PC_Gui.o: PC_Gui.cpp PC_Gui.hpp GuiCallbacks.cpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags gtk+-3.0) -c $<

clean:
	$(RM) *.o *.gch $(TARGET)
