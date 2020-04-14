CC = gcc
EXEC = ifindex
OBJS = ifindex.o
FILE = ifindex.c

all: $(EXEC)
	$(CC) -g $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./ifindex
