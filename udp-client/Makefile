CC = gcc -g -m64
EXEC = udp-client
OBJS = udp-client.o
FILE = udp-client.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./udp-client
