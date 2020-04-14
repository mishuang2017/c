CC = gcc -g -m64
EXEC = my_netlink
OBJS = my_netlink.o
FILE = my_netlink.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
#	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./my_netlink
