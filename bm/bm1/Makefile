FILE2 = bm1.s 

all: bm1

bm1: ${FILE2} Makefile
	$(CC) $(CFLAGS) -static ${FILE2} -Os -o bm1.out -w

clean:
	rm -rf bm1.out *.o
