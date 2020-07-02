CC=gcc
CFLAGS= -std=c++14
LFLAGS= -lstdc++ $$(pkg-config --libs portaudio-2.0)
TARGET= transmitter

all: $(TARGET)

$(TARGET): transmitter.o
	$(CC) $(LFLAGS) $^ -o $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

run: clean $(TARGET)
	@ echo '================================'; ./$(TARGET); RET=$$?; echo '================================'; echo EXIT_CODE $$RET;

clean:
	$(RM) *.o $(TARGET)
