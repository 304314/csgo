FILE2 = qrencode.s qrframe.s qrtest.s support/beebsc.s support/main.s support/chipsupport.s support/boardsupport.s

all: qrduino

qrduino: ${FILE2} Makefile
	$(CC) $(CFLAGS) -static ${FILE2} -Os -o qrduino -I support

clean:
	rm -rf qrduino *.o
