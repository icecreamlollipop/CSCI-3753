
# the compiler: gcc for C program
CC = gcc

# compiler flags:
CFLAGS = -pthread -Wall -Wextra

# the build target executable:
TARGET = multi-lookup

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(TARGET).c util.c -o $(TARGET) $(CFLAGS)

clean:
	$(RM) $(TARGET)