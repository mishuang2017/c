CC = gcc -g -m64
EXEC = udp-server
OBJS = udp-server.o
FILE = udp-server.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./udp-server
