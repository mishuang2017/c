CC = gcc -g -m64
EXEC = va_start
OBJS = va_start.o
FILE = va_start.c

all: $(EXEC)
	$(CC) $(FILE) -fsanitize=address -o $(EXEC)
#	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./va_start
