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
		rcp $(BIN2) root@cloud-mwm-11.us.oracle.com:/root;	\
		rcp $(BIN2) root@cloud-mwm-12.us.oracle.com:/root;	\
	else								\
		$(ECHO) i386;						\
		$(GCC) -lnsl -lsocket -o $(BIN) $(BIN).c;		\
		$(GCC) -lnsl -lsocket -o $(BIN2) -DSDP $(BIN).c;	\
		rcp $(BIN2) root@cloud-mwm-11.us.oracle.com:/root;	\
		rcp $(BIN2) root@cloud-mwm-12.us.oracle.com:/root;	\
		rcp $(BIN) root@cloud-mwm-11.us.oracle.com:/root;	\
		rcp $(BIN) root@cloud-mwm-12.us.oracle.com:/root;	\
	fi

clean:
	@rm $(BIN) $(BIN2)
