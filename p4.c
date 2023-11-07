#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define EMPTY 0
#define RED 1
#define YELLOW 2

typedef struct {
	uint64_t a;
	uint64_t b;
} Board;

int get_val(Board *bo, int col, int row) {
	uint64_t temp = bo->a;
	if (col >= 4) {
		temp = bo->b;
		col -= 4;
	}

	return (temp >> (2 * (col * 6 + row))) & 3;
}

void set_val(Board *bo, int col, int row, uint64_t val) {
	uint64_t *temp;
	uint64_t decal;
	printf("set %d on %d,%d\n",val,row,col);
	if (col >= 4) {
		temp = &bo->b;
		decal = (2 * ((col - 4) * 6 + row));
	} else {
		temp = &bo->a;
		decal = (2 * (col * 6 + row));
	}

	printf("decal = %d\n",decal);
	uint64_t trois = 4-1;
	*temp &= ~(trois << decal);
	*temp |= val << decal;
}


void print_board(Board *bo) {
	for (int row=5;row >= 0; row--) {
		printf("| ");
		for (int col=0;col<7;col++) {
			switch  (get_val(bo,col,row)) {
				case EMPTY : printf(" "); break;
				case RED   : printf("X"); break;
				case YELLOW: printf("O"); break;
				default : printf("ERROR on %d,%d\n",col,row);
			}
			printf(" | ");
		}
		putchar('\n');
	}
}

int insert(Board *bo, int col, int color) {
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			return 0;
		}
	}
	if (row == 6) {
		printf("Column %d full\n",col);
		return 1;
	}
}


//Check probablement lent, à améliorer
int win_check(Board *bo, int color) {
	int draw = 2;
	for (int col=0;col<7;col++) {
		for (int row=0;row<6;row++) {
			if (get_val(bo,col,row) == color) {
				if (row < 3) {
					if (get_val(bo,col,row+1) == color && get_val(bo,col,row+2) == color && get_val(bo,col,row+3) == color) {
						printf("col, %d %d\n",col,row);
						return 1;
					}
				}

				if (col < 4) {
					if (get_val(bo,col+1,row) == color && get_val(bo,col+2,row) == color && get_val(bo,col+3,row) == color){
						printf("row, %d %d\n",col,row);
						return 1;				
					}
				}

				if (col < 4 && row < 3) {
					if (get_val(bo,col+1,row+1) == color && get_val(bo,col+2,row+2) == color && get_val(bo,col+3,row+3) == color){
						printf("diag++, %d %d\n",col,row);
						return 1;					
					}
				}
				if (col <4 && row > 2) {
					if (get_val(bo,col+1,row-1) == color && get_val(bo,col+2,row-2) == color && get_val(bo,col+3,row-3) == color) {
						printf("diag+-, %d %d\n",col,row);
						return 1;					
					}

				}
			} else if (get_val(bo,col,row) == EMPTY) {
				draw = 0;
			}
		}
	}
	return draw;
}



int main() {
	uint64_t val = 0x0000000000000000;
	Board bo;
	bo.a = 0;
	bo.b = 0;
	int current_color = RED;

	print_board(&bo);
	int col;
	while (1) {
		scanf("%d",&col);
		insert(&bo,col,current_color);
		print_board(&bo);
		if (win_check(&bo,current_color)) {
			printf("Bien joué !\n");
			printf("%lx %lx\n",bo.a,bo.b);
			exit(0);
		}
		if (current_color == RED) {
			current_color = YELLOW;
		} else {
			current_color = RED;
		}


	}
}