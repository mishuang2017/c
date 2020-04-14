CC = gcc
EXEC = memchr
OBJS = memchr.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./memchr
