CC     = g++
CFLAGS = -Wall -Wextra -O2
LIBS   = -lm

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

all: $(OBJ)
	$(CC) $(OBJ) -o tmg-wall-alt $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tmg-wall-alt

