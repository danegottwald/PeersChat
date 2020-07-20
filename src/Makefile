CC= gcc
INCLUDE= -I Network -I Audio -I GUI
CFLAGS= -std=c++14 -Wall -Wextra -pedantic -Wpedantic -O2 $(INCLUDE)
LFLAGS= -lstdc++ $$(pkg-config --libs portaudio-2.0 opus gtk+-3.0)
TARGET= PeersChat

all: $(TARGET) tidy

$(TARGET): $(TARGET).o PC_Audio.o PC_Network.o PC_Gui.o GuiCallbacks.o
	$(CC) $^ -o $(TARGET) $(LFLAGS)

Audio: PC_Audio.o
Network: PC_Network.o
GUI: PC_Gui.o GuiCallbacks.o

$(TARGET).o: $(TARGET).cpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags opus portaudio-2.0 gtk+-3.0) -c $<

PC_Audio.o: ./Audio/PC_Audio.cpp ./Audio/PC_Audio.hpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags opus portaudio-2.0) -c $<

PC_Network.o: ./Network/PC_Network.cpp ./Network/PC_Network.hpp
	$(CC) $(CFLAGS) -c $<

PC_Gui.o: ./GUI/PC_Gui.cpp ./GUI/PC_Gui.hpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags gtk+-3.0 opus) -c $<

GuiCallbacks.o: ./GUI/GuiCallbacks.cpp ./GUI/GuiCallbacks.hpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags gtk+-3.0 opus) -c $<

tidy:
	$(RM) $$(find . -type f -name '*.o')

clean: tidy
	$(RM) $(TARGET)

