NAME := ft_traceroute

CC := cc
CFLAGS := -Wall -Wextra -Werror
INCLUDES := -Iincludes

SRCS := \
	src/main.c \
	src/args.c \
	src/resolve.c \
	src/trace.c \
	src/print.c \
	src/util.c

OBJDIR := obj
OBJS := $(SRCS:src/%.c=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
