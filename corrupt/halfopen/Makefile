ARCH	:sh = (uname -p)
SUN_CC=/ws/on11update-tools/SUNWspro/sunstudio12.1/bin/cc
GCC=gcc

BIN=halfopen
BIN2=$(BIN)_sdp

ECHO=echo

all: $(BIN).c
# 	$(CC) -lnsl -lsocket -o $(BIN) $(BIN).c
	@if [ "$(ARCH)" = sparc ]; then					\
		$(ECHO) sparc;						\
		$(SUN_CC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
		$(SUN_CC) -lnsl -lsocket -o $(BIN) $(BIN).c;		\
		rcp $(BIN) root@wave221.us:/root;			\
		rcp $(BIN) root@wave228.us:/root;			\
		rcp $(BIN2) root@wave221.us:/root;			\
		rcp $(BIN2) root@wave228.us:/root;			\
	else								\
		$(ECHO) i386;						\
		$(GCC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
		rcp $(BIN2) root@sand100.us:/root;			\
		rcp $(BIN2) root@sand101.us:/root;			\
	fi

clean:
	@rm $(BIN)
	@rm $(BIN2)
