CC = gcc
EXEC = test
OBJS = test.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./test
