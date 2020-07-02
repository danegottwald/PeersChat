CC=gcc
CFLAGS= -std=c++14
LFLAGS= -lstdc++ $$(pkg-config --libs portaudio-2.0)
TARGET= receiver

all: $(TARGET)

$(TARGET): receiver.o
	$(CC) $(LFLAGS) $^ -o $(TARGET)

receiver.o: receiver.cpp
	$(CC) $(CFLAGS) -c $<

run: clean $(TARGET)
	@ echo '================================'; ./$(TARGET); RET=$$?; echo '================================'; echo EXIT_CODE $$RET;

clean:
	$(RM) *.o $(TARGET)
