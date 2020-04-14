CC = gcc -g -m64
EXEC = asm
OBJS = asm.o
FILE = asm.c

all: $(EXEC)
	$(CC) -g $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./asm
