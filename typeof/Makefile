CC = gcc -g -m64
EXEC = typeof
OBJS = typeof.o
FILE = typeof.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./typeof
