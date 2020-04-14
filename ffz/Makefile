CC = gcc
EXEC = ffz
OBJS = ffz.o
FILE = ffz.c

all: $(EXEC)
	$(CC) -g $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./ffz
