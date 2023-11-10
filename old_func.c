#include <stdio.h>

#include "old_func.h"

int insert(Board *bo, int col, int color) {
	if (col > 6) {
		printf("col trop grande\n");
		return 1;
	}
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			bo->nb_pions++;
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
//////////////////////////

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
			if (score <= beta)
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
			if (score >= alpha)
				return score;

			if (score == 1)
				return score;
		}
	}

	// printf("-> max, depth : %d, score : %d\n",depth,score);
	return score;
}


int cout_coup(Board *bo,int current_color, int* res) {

	if (MULTITHREADING == 1) {
//Multithreaded version
		pthread_t threads[7];
		struct search_args args[7];

		for (int i=0;i<7;i++) {

			args[i].bo = bo;
			args[i].i = i;
			args[i].current_color = current_color;
			args[i].res = res;

			pthread_t thread_id;
			pthread_create(&thread_id, NULL, &start_search, (void *)&args[i]); 
			threads[i] = thread_id;
		}
		for (int i=0;i<7;i++) {
			pthread_join(threads[i], NULL); 
		}

		int score = res[0];
		int coup = 0;
		for (int i=1;i<7;i++) {
			if (res[i] == 2) continue;
			if (current_color == RED) {
				if (res[i] > score) {
					score = res[i];
					coup = i;
				}
			} else {
				if (res[i] > score) {
					score = res[i];
					coup = i;
				}
			}
		}
		return coup;
	} else {
//Single threaded version
		if (current_color == RED) {
			int score = -2;
			int coup;

			for (int col=0;col<7;col++) {
				Board temp = *bo;
				if (insert(&temp,col,RED)) continue;
				int val = min(&temp,0,-2);
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
				int val = max(&temp,0,2);
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
}

