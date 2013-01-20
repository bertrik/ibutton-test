CC = gcc
PROF = 
DEBUG = -g
O_FLAGS = -O2
C_FLAGS = $(O_FLAGS) -Wall $(DEBUG) $(PROF)
L_FLAGS = $(O_FLAGS) $(DEBUG) $(PROF)

O_FILES = ds1961_sha.o hexutil.o main.o

makehash: $(O_FILES)
	$(CC) $(L_FLAGS) -o makehash $(O_FILES)

hexutil.o: ../hexutil.c
	$(CC) -c $(C_FLAGS) $<

.c.o:	ds1961_sha.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f makehash *.o core
