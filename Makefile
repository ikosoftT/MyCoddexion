NAME = codexion
CC = cc 
CFLAGS =  -Wall -Wextra -Werror -pthread
SRC = \
	src/main.c \
	src/init.c \
	src/dongles.c \
	src/monitor.c \
	src/parser.c \
	src/thread.c \
	src/utils.c \
	src/logger.c \
	src/simulation.c \
	src/scheduler.c \
	src/heap.c \
	src/heap_utils.c \
	src/cleanup.c \
	src/thread_time_init.c \
	src/scheduler_utils.c \
	src/monitor_util.c \

OBJ = $(SRC:.c=.o)

HR = src/codexion.h

all: $(NAME) clean

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c $(HR)
	$(CC) $(CFLAGS) -c  $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
