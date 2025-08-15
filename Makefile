CC     = gcc
CFLAGS = -Wall -Wextra -ggdb
LIBS   = -lm

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

all: $(OBJ)
	$(CC) $(OBJ) -o tmg-wall $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tmg-wall

