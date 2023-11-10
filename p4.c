#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define EMPTY 0
#define RED 1
#define YELLOW 2

#define MULTITHREADING 1
#define INTERACTIVE 2
#define MAX_DEPTH 11

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

int insert_checkwin(Board *bo, const int col, const int color, int *score,Board *** PionsMask,int *winordraw,const int NbPions){
	if (col > 6) {
		printf("col trop grande\n");
		return 1;
	}
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			break;
		}
	}
	if (row == 6) {
		// printf("Column %d full\n",col);
		return 1;
	}
	
	int res = color==RED?1:-1;
	int draw = 1;
	int p=0;
	
	*winordraw =0;
	Board TestBoard;
	if(color==RED){
		while((PionsMask[col][row][p].a>0) || (PionsMask[col][row][p].b>0)){
			TestBoard.a=PionsMask[col][row][p].a & bo->a;
			TestBoard.b=PionsMask[col][row][p].b & bo->b;
			if((TestBoard.a==PionsMask[col][row][p].a)&&(TestBoard.b==PionsMask[col][row][p].b)){
				*winordraw =1;
				*score = res;
				return 0;
			}
			p++;
		}
	}
	else{
		while((PionsMask[col][row][p].a>0) || (PionsMask[col][row][p].b>0)){
			TestBoard.a=(PionsMask[col][row][p].a<<1) & bo->a;
			TestBoard.b=(PionsMask[col][row][p].b<<1) & bo->b;
			if((TestBoard.a==(PionsMask[col][row][p].a<<1))&&(TestBoard.b==(PionsMask[col][row][p].b<<1))){
				*winordraw =1;
				*score = res;
				return 0;
			}
			p++;
		}
	}
	if(NbPions+1==42){
		*winordraw =1;
		*score = 0;
		return 0;
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
////////// ALTERNATIVE avec masques //////////
int insert2(Board *bo, int col, int color,int* rowrec ) {
	if (col > 6) {
		printf("col trop grande\n");
		return 1;
	}
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			*rowrec = row;
			return 0;
		}
	}
	if (row == 6) {
		// printf("Column %d full\n",col);
		return 1;
	}
}

int win_check2(Board *bo, int color,int col,int row, int *score, Board *** PionsMask,const int NbPions) {
	int res = color==RED?1:-1;
	int draw = 1;
	int p=0;

	Board TestBoard;
	if(NbPions+1==42){
		*score = 0;
		return 1;
	}
	if(color==RED){
		while((PionsMask[col][row][p].a>0) || (PionsMask[col][row][p].b>0)){
			TestBoard.a=PionsMask[col][row][p].a & bo->a;
			TestBoard.b=PionsMask[col][row][p].b & bo->b;
			if((TestBoard.a==PionsMask[col][row][p].a)&&(TestBoard.b==PionsMask[col][row][p].b)){
				*score = res;
				return 1;
			}
			p++;
		}
	}
	else{
		while((PionsMask[col][row][p].a>0) || (PionsMask[col][row][p].b>0)){
			TestBoard.a=(PionsMask[col][row][p].a<<1) & bo->a;
			TestBoard.b=(PionsMask[col][row][p].b<<1) & bo->b;
			if((TestBoard.a==(PionsMask[col][row][p].a<<1))&&(TestBoard.b==(PionsMask[col][row][p].b<<1))){
				*score = res;
				return 1;
			}
			p++;
		}
	}

	return 0;
}



///////////////

volatile uint64_t N = 0;



//////////////////////////

int max2(Board *bo,int depth,int alpha,Board *** PionsMask,int NbPions,const int colR,const int row);
int min2(Board *bo,int depth,int beta,Board *** PionsMask,int NbPions,const int colR,const int row) { //Yellow to play
	int score = 2;
	// printf("min, depth : %d\n",depth);
	// print_board(bo);

	N++;
	
	if (win_check2(bo,RED,colR,row,&score,PionsMask,NbPions)) {
		return score;
	}

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		int rowtemp;
		if (insert2(&temp,col,YELLOW,&rowtemp)) continue;
		int val = max2(&temp,depth+1,score,PionsMask,NbPions+1,col,rowtemp);
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

int max2(Board *bo,int depth,int alpha,Board *** PionsMask,int NbPions,const int colR,const int row) { //Red to play
	// printf("max, depth : %d\n",depth);
	// print_board(bo);

	N++;

	if (depth>MAX_DEPTH) return 0;
	int score = -2;

	if (win_check2(bo,YELLOW,colR,row,&score,PionsMask,NbPions)) {
		return score;
	}

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		int rowtemp;
		if (insert2(&temp,col,RED,&rowtemp)) continue;
		int val = min2(&temp,depth+1,score,PionsMask,NbPions+1,col,rowtemp);
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

int cout_coup2(Board *bo,int current_color, int* res,Board *** PionsMask,int NbPions) {
	if (current_color == RED) {
		int score = -2;
		int coup;

		int winordraw;
		for (int col=0;col<7;col++) {
			Board temp = *bo;
			int rowtemp;
			if (insert2(&temp,col,RED,&rowtemp)) continue;
			//if(winordraw) return score; 
			int val = min2(&temp,0,-2,PionsMask,NbPions+1,col,rowtemp);
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
		int winordraw;
		for (int col=0;col<7;col++) {
			Board temp = *bo;
			int rowtemp;
			if (insert2(&temp,col,YELLOW,&rowtemp)) continue;
			int val = max2(&temp,0,2,PionsMask,NbPions+1,col,rowtemp);
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

struct search_args {
	Board *bo;
	int i;
	int current_color;
	int *res;
};


void *start_search(void *arg) {
	struct search_args *a = (struct search_args*)arg;

	Board temp = *a->bo;
	if (insert(&temp,a->i,RED)) {
		a->res[a->i] = 2;
	}

	int val;
	if (a->current_color == RED) {
		val = min(&temp,0,-2); //-2 to not have alpha beta pruning
	} else {
		val = max(&temp,0,2); //same
	}

	printf("	%d : %d\n",a->i,val);
	a->res[a->i] = val;

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




int InitMask(Board *** PionsMask){
	for(int i=0;i<7;i++)
		PionsMask[i] =  (Board**)malloc(6*sizeof(Board*));
	Board boV[3][7]={0};
	Board boH[4][6]={0};
	//DP pour diagonale positive 
	//DM pour diagonale moins (negative)
	Board boDP[4][3]={0};
	Board boDM[4][3]={0};
//Creation des masques
	//Les masques verticaux (4)
	for(int i=0;i<7;i++){
		for(int k=0;k<3;k++){
			for(int j=0;j<4;j++){
				set_val(&boV[k][i],i,k+j,RED);
			}
		}
	}
	//Les masques  horizontaux(3)
	for(int i=0;i<6;i++){
		for(int k=0;k<4;k++){
			for(int j=0;j<4;j++){
				set_val(&boH[k][i],k+j,i,RED);
			}
		}
	}
	//Les masques diagonaux 
		
	for(int i=0;i<3;i++){
		for(int k=0;k<4;k++){
			for(int j=0;j<4;j++){
				//De Haut gauche a Bas droit
				set_val(&boDP[k][i],k+j,i+j,RED);
				//De Bas Gauche a Haut Droit
				set_val(&boDM[k][i],3+k-j,i+j,RED);
			}
		}
	}		
		
		
	printf("Masque crées");

//Affichage des masques 
/*
	for(int i=0;i<7;i++)
		for(int k=0;k<3;k++) {
			print_board(&boV[k][i]);
			printf("\n");
		}
	for(int i=0;i<6;i++)
		for(int k=0;k<4;k++) {
			print_board(&boH[k][i]);
			printf("\n");
		}

	for(int i=0;i<3;i++){
		for(int k=0;k<4;k++){
				//De Haut gauche a Bas droit
				print_board(&boDM[k][i]);
				printf("\n");
		}
	}		
	for(int i=0;i<3;i++){
		for(int k=0;k<4;k++){
				//De Haut gauche a Bas droit
				print_board(&boDP[k][i]);
				printf("\n");
		}
	}		
*/		
	Board TempMask[13]={0};
	for(int col=0;col<7;col++){
		for(int row=0;row<6;row++){
			int cptNbmask =0;
			//parcours tous les masques : 
			for(int i=0;i<7;i++)
				for(int k=0;k<3;k++) {
					if (get_val(&boV[k][i],col,row)){
						//printf("Le pion : %d,%d a pour masque : boV %d,%d\n",col,row,k,i );
						TempMask[cptNbmask]=boV[k][i];
						cptNbmask++;
						
					}
				}
			for(int i=0;i<6;i++)
				for(int k=0;k<4;k++) {
					if (get_val(&boH[k][i],col,row)){
						//printf("Le pion : %d,%d a pour masque : boH %d,%d\n",col,row,k,i );
						TempMask[cptNbmask]=boH[k][i];
						cptNbmask++;
					}
				}
				

			for(int i=0;i<3;i++){
				for(int k=0;k<4;k++){
						//De Haut gauche a Bas droit
						if (get_val(&boDP[k][i],col,row)){
							//printf("Le pion : %d,%d a pour masque : boDP %d,%d\n",col,row,k,i );
							TempMask[cptNbmask]=boDP[k][i];
							cptNbmask++;
						}
				}
			}		
			for(int i=0;i<3;i++){
				for(int k=0;k<4;k++){
						//De Haut gauche a Bas droit
						if(get_val(&boDM[k][i],col,row)){
							//printf("Le pion : %d,%d a pour masque : boDM %d,%d\n",col,row,k,i );
							TempMask[cptNbmask]=boDM[k][i];
							cptNbmask++;
					}
				}
			}				
			/*for(int i=0;i<12;i++){
				print_board(&TempMask[i]);
			}*/
			
			//fin de l'explo de tous les masques 
			PionsMask[col][row]=(Board*)malloc(13*sizeof(Board));
			memset(PionsMask[col][row],0,13*sizeof(Board));
			memcpy(PionsMask[col][row],TempMask,13*sizeof(Board));
			memset(TempMask,0,13*sizeof(Board));
			
		}
	}
	return 1;
}
int freeMask(Board *** PionsMask){
	for(int col=0;col<7;col++)
		for(int row=0;row<6;row++)
			free(PionsMask[col][row]);
	return 1;
}








int main() {
	uint64_t val = 0x0000000000000000;
	Board bo;
	bo.a = 0;
	bo.b = 0;
	int current_color = RED;

	print_board(&bo);

	if (INTERACTIVE == 1) {

		int col;
		int winordraw;
		int NbPions = 0;
		Board *** PionsMask = (Board***)malloc(7*sizeof(Board**));
		InitMask(PionsMask);

		while (1) {
			int scores[7];
			int score;
			int NbPions = 0;
			
			clock_t start, end;
			
			N = 0;
			struct timeval start_i;
			gettimeofday(&start_i,NULL);
			printf("Lancement ...\n");
			start = clock();
			
			int coup = cout_coup2(&bo, RED, &score,PionsMask,NbPions);
			NbPions++;
			
			end = clock();

			struct timeval end_i;
			gettimeofday(&end_i,NULL);

			double time = (end - start) / (double)CLOCKS_PER_SEC;

			printf("Wall time : %d,%ds\n",end_i.tv_sec - start_i.tv_sec);
			printf("calculated %lu nodes in %fs (%e nodes/s)\n",N,time,N/time);

			printf("played %d\n",coup);
			insert(&bo, coup,RED);

			print_board(&bo);
			
			if (win_check(&bo, RED, &score)) exit(0);

			int col;
			scanf("%d",&col);
			insert(&bo, col,YELLOW);
			print_board(&bo);
			if (win_check(&bo, YELLOW, &score)) exit(0);
		}
		
	} else if (INTERACTIVE == 2){
		while (1) {
			int scores[7];
			int score;

			clock_t start, end;

			N = 0;
			struct timeval start_i;
			gettimeofday(&start_i,NULL);
			start = clock();
			int coup = cout_coup(&bo, RED, scores);
			end = clock();

			struct timeval end_i;
			gettimeofday(&end_i,NULL);

			double time = (end - start) / (double)CLOCKS_PER_SEC;

			printf("Wall time : %d,%ds\n",end_i.tv_sec - start_i.tv_sec);
			printf("calculated %lu nodes in %fs (%e nodes/s)\n",N,time,N/time);

			printf("played %d\n",coup);
			insert(&bo, coup,RED);

		 	print_board(&bo);
			
			if (win_check(&bo, RED, &score)) exit(0);

			int col;
			scanf("%d",&col);
			insert(&bo, col,YELLOW);
			print_board(&bo);
			if (win_check(&bo, YELLOW, &score)) exit(0);


		}
	} else {

		int scores[7];

		clock_t start, end;

		N = 0;
		struct timeval start_i;
		gettimeofday(&start_i,NULL);
		start = clock();
		int coup = cout_coup(&bo, RED, scores);
		end = clock();

		struct timeval end_i;
		gettimeofday(&end_i,NULL);

		double time = (end - start) / (double)CLOCKS_PER_SEC;

		printf("Wall time : %d,%ds\n",end_i.tv_sec - start_i.tv_sec);
		printf("calculated %lu nodes in %fs (%e nodes/s)\n",N,time,N/time);

		printf("%d\n",coup);

	}
}