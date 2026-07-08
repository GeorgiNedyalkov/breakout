W64DEVKIT   := C:/raylib/w64devkit/bin
RAYLIB_SRC  := C:/raylib/raylib/src

CC      := $(W64DEVKIT)/gcc.exe
CFLAGS  := -I$(RAYLIB_SRC) -Wall
LDFLAGS := -L$(RAYLIB_SRC) -lraylib -lopengl32 -lgdi32 -lwinmm -B$(W64DEVKIT)

TARGET := main.exe
SRC    := main.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
