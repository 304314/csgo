FILE2 = rnd_output0.s  rnd_output1.s  rnd_output2.o  rnd_output3.o 

all: rnd

rnd: ${FILE2} Makefile
	$(CC) $(CFLAGS) -static ${FILE2} -Os -o rnd.out

clean:
	rm -rf rnd.out 
