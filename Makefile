# _*_ MakeFile _*_

CC = g++

CFLAGS = -Wall -Wextra -Werror -std=c++17 -lpthread

SERVICE = service
CLI = cli

SRC_SERVICE = main.cpp Neighbor.cpp
SRC_CLI = cli.cpp Neighbor.cpp

all: $(SERVICE) $(CLI)

$(SERVICE): $(SRC_SERVICE)
	$(CC) $(CFLAGS) $(SRC_SERVICE) -o $(SERVICE)

$(CLI): $(SRC_CLI)
	$(CC) $(CFLAGS) $(SRC_CLI) -o $(CLI)

fclean:
	rm -f $(SERVICE) $(CLI)

re: fclean all

.PHONY: all fclean re