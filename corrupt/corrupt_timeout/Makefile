ARCH	:sh = (uname -p)
SUN_CC=/ws/on11update-tools/SUNWspro/sunstudio12.1/bin/cc
GCC=gcc

BIN=corrupt
BIN2=$(BIN)_sdp

ECHO=echo

all:
	@if [ "$(ARCH)" = sparc ]; then					\
		$(ECHO) sparc;						\
		$(SUN_CC) -lnsl -lsocket -o $(BIN) $(BIN).c;		\
		$(SUN_CC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
		rcp corrupt_sdp root@ip-10-134-78-122.us.oracle.com:/root;		\
		rcp corrupt_sdp root@ip-10-134-78-123.us.oracle.com:/root;		\
# 		rcp corrupt root@ip-10-134-78-122.us.oracle.com:/root;		\
# 		rcp corrupt root@ip-10-134-78-123.us.oracle.com:/root;		\
	else								\
		$(ECHO) i386;						\
		$(GCC) -lnsl -lsocket -o $(BIN) $(BIN).c;		\
		$(GCC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
		rcp $(BIN) $(BIN2) root@sand100.us:/root;			\
		rcp $(BIN) $(BIN2) root@sand101.us:/root;			\
	fi

clean:
	@rm $(BIN) $(BIN2)
