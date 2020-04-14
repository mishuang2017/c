CC = gcc
EXEC = iphdr
OBJS = iphdr.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	./iphdr
