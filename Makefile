CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lncurses

SRC = main.c manager.c
OBJ = $(SRC:.c=.o)
EXEC = lp25

all: $(EXEC)
$(EXEC): $(OBJ)
$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

clean:
rm -f $(OBJ) $(EXEC)
