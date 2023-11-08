C_ARGS= -O3 -g

p4: p4.c
	gcc p4.c -o p4 $(C_ARGS)

clean:
	rm p4