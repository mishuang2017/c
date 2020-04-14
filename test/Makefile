CC = g++
EXEC = test
OBJS = test.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)  -O0  -g

clean:
	rm -f $(EXEC) *.elf *.gdb *.o core.*

run:
	./test
