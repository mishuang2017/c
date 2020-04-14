CC = gcc -g -m64
EXEC = vxlan-port
OBJS = vxlan-port.o
FILE = vxlan-port.c

all: $(EXEC)
	$(CC) $(FILE) -o $(EXEC)
# 	$(CC) -Wall -Werror -ansi -pedantic-errors -g $(FILE) -o $(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./vxlan-port
