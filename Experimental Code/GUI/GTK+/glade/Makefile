CC=g++
CFLAGS= -rdynamic -std=c++14 `pkg-config --cflags gtk+-3.0`
LIBS= `pkg-config --libs gtk+-3.0` -export-dynamic
TARGET= demo
all: $(TARGET)

$(TARGET): demo.o
	$(CC) $^ -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< 

run: clean $(TARGET)
	@ echo '================================'; ./$(TARGET); RET=$$?; echo '================================'; echo EXIT_CODE $$RET;

clean:
	$(RM) *.o $(TARGET)
