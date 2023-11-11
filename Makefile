C_ARGS= -O3 -g -flto

p4: p4.c Board.h  Board.o
	gcc p4.c Board.o -o p4 $(C_ARGS)

Board.o : Board.h Board.c
	gcc Board.c -c -o Board.o $(C_ARGS)

clean:
	rm p4 Board.o
