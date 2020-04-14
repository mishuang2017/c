ARCH	:sh = (uname -p)
SUN_CC=/ws/on11update-tools/SUNWspro/sunstudio12.1/bin/cc
GCC=gcc

BIN=corrupt
BIN2=$(BIN)_sdp

ECHO=echo

all:
	@if [ "$(ARCH)" = sparc ]; then					\
		$(ECHO) sparc;						\
		$(SUN_CC) -lnsl -lpthread -o $(BIN) $(BIN).c;		\
		$(SUN_CC) -lnsl -lpthread -o $(BIN2) -DSDP $(BIN).c;	\
	else								\
		$(ECHO) i386;						\
		$(GCC) -lnsl -lpthread -o $(BIN) $(BIN).c;		\
		$(GCC) -lnsl -lpthread -o $(BIN2) -DSDP $(BIN).c;	\
	fi

clean:
	@rm $(BIN) $(BIN2)
