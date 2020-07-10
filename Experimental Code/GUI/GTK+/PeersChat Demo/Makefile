CC=g++
CFLAGS= -std=c++14 `pkg-config --cflags gtk+-3.0`
LIBS= `pkg-config --libs gtk+-3.0`
TARGET= demogui
all: $(TARGET)

$(TARGET): demogui.o
	$(CC) $^ -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< 

run: clean $(TARGET)
	@ echo '================================'; ./$(TARGET); RET=$$?; echo '================================'; echo EXIT_CODE $$RET;

clean:
	$(RM) *.o $(TARGET)
