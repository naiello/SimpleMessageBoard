CC = gcc
FLAGS = -Wall

all: myfrmd

myfrmd: userhash.o main.o ../common/transfer.o cmds.o
	$(CC) $(FLAGS) userhash.o main.o ../common/transfer.o cmds.o -o myfrmd

userhash.o: userhash.c userhash.h ../common/transfer.h
	$(CC) $(FLAGS) -c userhash.c 

main.o: main.c userhash.h ../common/transfer.h
	$(CC) $(FLAGS) -c main.c

cmds.o: cmds.c cmds.h ../common/transfer.h
	$(CC) $(FLAGS) -c cmds.c

../common/transfer.o: ../common/transfer.c ../common/transfer.h
	$(CC) $(FLAGS) -c ../common/transfer.c -o ../common/transfer.o

.PHONY: clean
clean:
	rm -f *.o ../common/transfer.o myfrmd
