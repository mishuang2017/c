ARCH	:sh = (uname -p)
SUN_CC=/ws/on11update-tools/SUNWspro/sunstudio12.1/bin/cc
GCC=gcc

BIN=corrupt
BIN2=$(BIN)_sdp

ECHO=echo

# 		rcp $(BIN2) root@shiphead-ldom-6:/root;		\
# 		rcp $(BIN2) root@shiphead-ldom-7:/root;		\
# 		rcp $(BIN) root@shiphead-ldom-6:/root;		\
# 		rcp $(BIN) root@shiphead-ldom-7:/root;		\

all:
	@if [ "$(ARCH)" = sparc ]; then					\
		$(ECHO) sparc;						\
		$(SUN_CC) -lnsl -lsocket -o $(BIN) $(BIN).c;		\
		$(SUN_CC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
		rcp $(BIN2) root@ip-10-142-247-237.us:/root;		\
		rcp $(BIN2) root@ip-10-142-247-238.us:/root;		\
		rcp $(BIN) root@ip-10-142-247-237.us:/root;		\
		rcp $(BIN) root@ip-10-142-247-238.us:/root;		\
	else								\
		$(ECHO) i386;						\
		$(GCC) -lnsl -lsocket -o $(BIN) $(BIN).c;		\
		$(GCC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
	fi

clean:
	@rm $(BIN) $(BIN2)
