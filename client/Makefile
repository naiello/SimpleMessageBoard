# Nick Aiello [naiello], Rosalyn Tan [rtan]

all: myfrm

myfrm: client.c ../common/transfer.o
	gcc -Wall client.c ../common/transfer.o -o myfrm

../common/transfer.o: ../common/transfer.c
	gcc -Wall -c ../common/transfer.c -o ../common/transfer.o

.PHONY: clean
clean:
	rm -f *.o myfrm ../common/transfer.o
