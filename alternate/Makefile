CC     = g++
CFLAGS = -Wall -Wextra -ggdb
LIBS   = -lm

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

all: $(OBJ)
	$(CC) $(OBJ) -o tmg-wall-alt $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tmg-wall-alt

