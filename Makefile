CC		:= cc -g
CFLAGS	:= -Wall -Wextra -Werror -pthread
NAME	:= codexion
SRC_DIR	:= coders
SRC		:= main.c main_init.c threads.c threads_utils.c parsing.c dongles.c \
		dongles_lock.c dongles_helpers.c dongles_setup.c fifo_edf_utils.c \
		fifo_edf_utils_2.c fifo_edf_utils_3.c logs.c time.c monitor.c
SRCS	:= $(addprefix $(SRC_DIR)/, $(SRC))
OBJS	:= $(SRCS:.c=.o)
RM		:= rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

clean:
	$(RM) $(OBJS) a.out

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
