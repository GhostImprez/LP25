# Nom de l'executable
NAME = programme

# Dossiers
SRC_DIR = src
INC_DIR = include

# Fichiers sources
SRC = $(wildcard $(SRC_DIR)/*.c)

# Fichiers objets
OBJ = $(SRC:.c=.o)

# Options compilateur
CC = gcc
CFLAGS = -Wall -Wextra -I$(INC_DIR)

# bibliothèque ncurses
LDFLAGS = -lncurses

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(LDFLAGS)

# compiler les fichiers sources en fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# options de nettoyage
clean:
	rm -f $(OBJ) 
fclean: 
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re