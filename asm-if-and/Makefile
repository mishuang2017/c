CC = gcc -g -m64
EXEC = asm-if-and
OBJS = asm-if-and.o
FILE = asm-if-and.c

all: $(EXEC)
	$(CC) -g $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./asm-if-and
