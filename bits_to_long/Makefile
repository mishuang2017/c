CC = gcc
EXEC = bits_to_long
OBJS = bits_to_long.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./bits_to_long
