C_ARGS= -O3 -g

p4: p4.c Board.h  Board.o
	gcc p4.c Board.o -o p4 $(C_ARGS)

old_func.o: old_func.c old_func.h Board.h Board.o
	gcc old_func.c -c -o old_func.o $(C_ARGS)

Board.o : Board.h Board.c
	gcc Board.c -c -o Board.o $(C_ARGS)

clean:
	rm p4