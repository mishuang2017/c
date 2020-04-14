ARCH	:sh = (uname -p)
SUN_CC=/ws/on11update-tools/SUNWspro/sunstudio12.1/bin/cc
GCC=gcc

BIN=corrupt
BIN2=$(BIN)_sdp

ECHO=echo

all:
	$(ECHO) sparc;						\
	$(GCC)  -lnsl -lpthread  -o $(BIN) $(BIN).c;		\

clean:
	@rm $(BIN) $(BIN2)
