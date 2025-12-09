// Nom de l'executable
NAME = progamme

// Dossiers
SRC_DIR = src
INC_DIR = include

// Fichiers sources
SRC = $(wildcard $(SRC_DIR)/*.c)

// Fichiers objets
OBJ = $(SRC:.c=.o)


// Options compilateur
CC = gcc
CFLAGS = -Wall -Wextra -I$(INC_DIR)

// bibliothèque ncurses
LDFLAGS = -lncurses

all: $(NAME
$(NAME)): $(OBJ)
$(CC) $(OBJ) -o $(NAME) $(LDFLAGS)

// compiler les fichiers sources en fichiers objets
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) 
fclean: 
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re