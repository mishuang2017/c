CC = gcc
EXEC = inet_diag
OBJS = inet_diag.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core

run:
	sudo ./inet_diag
