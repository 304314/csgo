FILE2 = bm2_1.s  bm2_2.s  bm2_3.s  bm2_4.s 

all: bm2

bm2: ${FILE2} Makefile
	$(CC) $(CFLAGS) -static ${FILE2} -Os -o bm2.out

clean:
	rm -rf bm2.out *.o output*
