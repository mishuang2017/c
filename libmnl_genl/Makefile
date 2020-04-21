CC = gcc -g -m64 -lmnl
EXEC = libmnl_genl
OBJS = libmnl_genl.o
FILE = libmnl_genl.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
#	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./libmnl_genl
