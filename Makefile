CC      := gcc
LDFLAGS := -lssl -lcrypto -lpthread
TARGET  := hashcrack

all:
	$(CC) *.c -o $(TARGET) $(LDFLAGS)

clean:
	@ rm $(TARGET)

