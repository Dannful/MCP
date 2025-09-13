CC = gcc
CFLAGS = -lm -fopenmp -O3
TARGET = mcts
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET) $(ARGS)

.PHONY: all clean
