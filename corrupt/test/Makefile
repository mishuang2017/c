CC = cc
EXEC = test
OBJS = test.o

all: $(EXEC)

$(EXEC): $(OBJS)
	cc -m64 -c test.c
	cc -m64 -o test test.c

clean:
	rm -f $(EXEC) *.elf *.gdb *.o

run:
	./test
