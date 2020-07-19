CC= g++
CFLAGS= -std=c++14 -Wall -Werror -pedantic -Wpedantic -O2
LFLAGS= $$(pkg-config --libs portaudio-2.0 opus gtk+-3.0)
TARGET= PeersChat
PREFIX= ./src

all: $(TARGET) tidy

$(TARGET): $(TARGET).o PeersChat.o PC_Audio.o PC_Network.o PC_Gui.o
	$(CC) $(LFLAGS) $^ -o $(TARGET)

Audio: PC_Audio.o
Network: PC_Network.o
GUI: PC_Gui.o

$(TARGET).o: ./src/PeersChat.cpp
	$(CC) $(CFLAGS) -c $<

PC_Audio.o: ./src/Audio/PC_Audio.cpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags opus portaudio-2.0 gtk+-3.0) -c $<

PC_Network.o: ./src/Network/PC_Network.cpp 
	$(CC) $(CFLAGS) $$(pkg-config --cflags opus portaudio-2.0 gtk+-3.0) -c $<

#create GUI filename.o rule/build
PC_Gui.o: ./src/GUI/PC_Gui.cpp 
	$(CC) $(CFLAGS) $$(pkg-config --cflags opus portaudio-2.0 gtk+-3.0) -c $<

tidy:
	$(RM) $$(find . -type f -name '*.o')

clean: tidy
	$(RM) $(TARGET)

