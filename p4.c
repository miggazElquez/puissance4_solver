#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define EMPTY 0
#define RED 1
#define YELLOW 2

#define MAX_DEPTH 7

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

int insert(Board *bo, int col, int color) {
	if (col > 6) {
		printf("col trop grande\n");
		return 1;
	}
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			return 0;
		}
	}
	if (row == 6) {
		// printf("Column %d full\n",col);
		return 1;
	}
}


//Check probablement lent, à améliorer
int win_check(Board *bo, int color, int *score) {
	int res = color==RED?1:-1;
	int draw = 1;
	for (int col=0;col<7;col++) {
		for (int row=0;row<6;row++) {
			if (get_val(bo,col,row) == color) {
				if (row < 3) {
					if (get_val(bo,col,row+1) == color && get_val(bo,col,row+2) == color && get_val(bo,col,row+3) == color) {
						// printf("col, %d %d\n",col,row);
						*score = res;
						return 1;
					}
				}

				if (col < 4) {
					if (get_val(bo,col+1,row) == color && get_val(bo,col+2,row) == color && get_val(bo,col+3,row) == color){
						// printf("row, %d %d\n",col,row);
						*score = res;
						return 1;
					}
				}

				if (col < 4 && row < 3) {
					if (get_val(bo,col+1,row+1) == color && get_val(bo,col+2,row+2) == color && get_val(bo,col+3,row+3) == color){
						// printf("diag++, %d %d\n",col,row);
						*score = res;
						return 1;
					}
				}
				if (col <4 && row > 2) {
					if (get_val(bo,col+1,row-1) == color && get_val(bo,col+2,row-2) == color && get_val(bo,col+3,row-3) == color) {
						// printf("diag+-, %d %d\n",col,row);
						*score = res;
						return 1;
					}

				}
			} else if (get_val(bo,col,row) == EMPTY) {
				draw = 0;
			}
		}
	}
	if (draw) {
		*score = 0;
		return 1;
	}
	return 0;
}

int N = 0;

int max(Board *bo,int depth,int alpha);

int min(Board *bo,int depth,int beta) { //Yellow to play
	int score = 2;
	// printf("min, depth : %d\n",depth);
	// print_board(bo);

	N++;

	if (win_check(bo,RED,&score)) {
		return score;
	}

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		if (insert(&temp,col,YELLOW)) continue;
		int val = max(&temp,depth+1,score);
		if (val < score) {
			score = val;
			if (score < beta)
				return score;
			if (score == -1) {
				return score;
			}
		}
	}

	// printf("-> min, depth : %d, score : %d\n",depth,score);

	return score;

}

int max(Board *bo,int depth,int alpha) { //Red to play
	// printf("max, depth : %d\n",depth);
	// print_board(bo);

	N++;

	if (depth>MAX_DEPTH) return 0;
	int score = -2;

	if (win_check(bo,YELLOW,&score)) {
		return score;
	}

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		if (insert(&temp,col,RED)) continue;
		int val = min(&temp,depth+1,score);
		if (val > score) {
			score = val;
			if (score > alpha)
				return score;

			if (score == 1)
				return score;
		}
	}

	// printf("-> max, depth : %d, score : %d\n",depth,score);
	return score;
}

int cout_coup(Board *bo,int current_color, int* res) {
	if (current_color == RED) {
		int score = -2;
		int coup;


		for (int col=0;col<7;col++) {
			Board temp = *bo;
			if (insert(&temp,col,RED)) continue;
			int val = min(&temp,0,score);
			printf("	%d : %d\n",col,val);
			if (val > score) {
				score = val;
				coup = col;
			}
		}
		*res = score;
		return coup;

	} else {
		int score = 2;
		int coup;

		for (int col=0;col<7;col++) {
			Board temp = *bo;
			if (insert(&temp,col,RED)) continue;
			int val = max(&temp,0,score);
			printf("	%d : %d\n",col,val);
			if (val < score) {
				score = val;
				coup = col;
			}
		}
		*res = score;
		return coup;

	}
}

int main() {
	uint64_t val = 0x0000000000000000;
	Board bo;
	bo.a = 0;
	bo.b = 0;
	int current_color = RED;

	print_board(&bo);

	int col;
	int score;
	int coup = cout_coup(&bo, RED, &score);
	printf("%d\n",coup);
	// while (1) {

	// 	int score;

	// 	int coup = cout_coup(&bo, RED, &score);

	// 	printf("coup : %d, eval=%d\n",coup,score);
	// 	printf("%d\n",N);
	// 	N = 0;
	// 	insert(&bo,coup,RED);
	// 	print_board(&bo);


	// 	if (win_check(&bo,RED,&score)) {
	// 		printf("fin de partie !\n");
	// 		printf("%d\n",score);
	// 		printf("%lx %lx\n",bo.a,bo.b);
	// 		exit(0);
	// 	}


	// 	scanf("%d",&col);
	// 	insert(&bo,col,YELLOW);
	// 	print_board(&bo);

	// 	if (win_check(&bo,YELLOW,&score)) {
	// 		printf("fin de partie !\n");
	// 		printf("%d\n",score);
	// 		printf("%lx %lx\n",bo.a,bo.b);
	// 		exit(0);
	// 	}
	// }
}