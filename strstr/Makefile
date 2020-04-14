CC = gcc -g -m64
EXEC = strstr
OBJS = strstr.o
FILE = strstr.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./strstr
