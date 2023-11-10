#include <stdio.h>

#include "Board.h"




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
	// printf("set %d on %d,%d\n",val,row,col);
	if (col >= 4) {
		temp = &bo->b;
		decal = (2 * ((col - 4) * 6 + row));
	} else {
		temp = &bo->a;
		decal = (2 * (col * 6 + row));
	}

	// printf("decal = %d\n",decal);
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




