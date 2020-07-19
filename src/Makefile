CC= gcc
CFLAGS= -std=c++14 -Wall -Werror -pedantic -Wpedantic -O2
LFLAGS= $$(pkg-config --libs portaudio-2.0 opus gtk+-3.0)
TARGET= PeersChat
PREFIX= ./src

all: $(TARGET) tidy

$(TARGET): $(TARGET).o PC_Audio.o PC_Network.o GUI.o
	$(CC) $(LFLAGS) $^ -o $(TARGET)

Audio: PC_Audio.o
Network: PC_Network.o
GUI: #insert GUI filename.o here

$(TARGET).o: $(TARGET).cpp
	$(CC) $(CFLAGS) -c $<

PC_Audio.o: ./src/Audio/PC_Audio.cpp ./src/Audio/PC_Audio.hpp
	$(CC) $(CFLAGS) $$(pkg-config --cflags opus portaudio-2.0) -c $<

PC_Network.o: ./src/Network/PC_Network.cpp ./src/Network/PC_Network.hpp
	$(CC) $(CFLAGS) -c $<

#create GUI filename.o rule/build

tidy:
	$(RM) $$(find . -type f -name '*.o')

clean: tidy
	$(RM) $(TARGET)

