#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "Board.h"


#define MULTITHREADING 0
#define INTERACTIVE 1
#define MAX_DEPTH 11




////////// ALTERNATIVE avec masques //////////
int insert(Board *bo, int col, int color,int* rowrec ) {
	if (col > 6) {
		printf("col trop grande\n");
		return 1;
	}
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			*rowrec = row;
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

int win_check2(Board *bo, int color,int col,int row, int *score, Board *** PionsMask) {
	int res = color==RED?1:-1;
	int draw = 1;
	int p=0;

	Board TestBoard;
	if(bo->nb_pions+1==42){
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

int max(Board *bo,int depth,int alpha,Board *** PionsMask,const int colR,const int row);
int min(Board *bo,int depth,int beta,Board *** PionsMask,const int colR,const int row) { //Yellow to play
	int score = 2;
	// printf("min, depth : %d\n",depth);
	// print_board(bo);

	N++;
	
	if (win_check2(bo,RED,colR,row,&score,PionsMask)) {
		return score;
	}

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		int rowtemp;
		if (insert(&temp,col,YELLOW,&rowtemp)) continue;
		int val = max(&temp,depth+1,score,PionsMask,col,rowtemp);
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

int max(Board *bo,int depth,int alpha,Board *** PionsMask,const int colR,const int row) { //Red to play
	// printf("max, depth : %d\n",depth);
	// print_board(bo);

	N++;

	if (depth>MAX_DEPTH) return 0;
	int score = -2;

	if (win_check2(bo,YELLOW,colR,row,&score,PionsMask)) {
		return score;
	}

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		int rowtemp;
		if (insert(&temp,col,RED,&rowtemp)) continue;
		int val = min(&temp,depth+1,score,PionsMask,col,rowtemp);
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
	Board *** PionsMask;
};


void *start_search(void *arg) {
	struct search_args *a = (struct search_args*)arg;

	int row;
	Board temp = *a->bo;
	if (insert(&temp,a->i,RED,&row)) {
		a->res[a->i] = 2;
	}

	int val;
	if (a->current_color == RED) {
		val = min(&temp,0,-2,a->PionsMask,a->i,row); //-2 to not have alpha beta pruning
	} else {
		val = max(&temp,0,2,a->PionsMask,a->i,row); //same
	}

	printf("	%d : %d\n",a->i,val);
	a->res[a->i] = val;

}


int cout_coup(Board *bo,int current_color, int* res,Board *** PionsMask) {
	if (MULTITHREADING == 1) {
//Multithreaded version
		pthread_t threads[7];
		struct search_args args[7];

		for (int i=0;i<7;i++) {

			args[i].bo = bo;
			args[i].i = i;
			args[i].current_color = current_color;
			args[i].res = res;
			args[i].PionsMask = PionsMask;

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
	} else { //single thread
		if (current_color == RED) {
			int score = -2;
			int coup;

			for (int col=0;col<7;col++) {
				Board temp = *bo;
				int rowtemp;
				if (insert(&temp,col,RED,&rowtemp)) continue;
				int val = min(&temp,0,-2,PionsMask,col,rowtemp);
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
				int rowtemp;
				if (insert(&temp,col,YELLOW,&rowtemp)) continue;
				int val = max(&temp,0,2,PionsMask,col,rowtemp);
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
	Board TempMask[14]={0};
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
			PionsMask[col][row]=(Board*)malloc(14*sizeof(Board));
			memset(PionsMask[col][row],0,14*sizeof(Board));
			memcpy(PionsMask[col][row],TempMask,14*sizeof(Board));
			memset(TempMask,0,14*sizeof(Board));
			
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
	bo.nb_pions = 0;
	int current_color = RED;
	print_board(&bo);

	if (INTERACTIVE == 1) {

		int col;
		Board *** PionsMask = (Board***)malloc(7*sizeof(Board**));
		InitMask(PionsMask);


		while (1) {
			int scores[7];
			int score;
			
			int row;
			
			clock_t start, end;
			
			N = 0;
			struct timeval start_i;
			gettimeofday(&start_i,NULL);
			printf("Lancement ...\n");
			start = clock();
			
			int coup = cout_coup(&bo, RED, &score,PionsMask);

			
			end = clock();

			struct timeval end_i;
			gettimeofday(&end_i,NULL);

			double time = (end - start) / (double)CLOCKS_PER_SEC;

			printf("Wall time : %ds\n",end_i.tv_sec - start_i.tv_sec);
			printf("calculated %lu nodes in %fs (%e nodes/s)\n",N,time,N/time);

			printf("played %d\n",coup);
			printf("nb_pions:%d\n",bo.nb_pions);
			insert(&bo, coup,RED,&row);


			print_board(&bo);
			
			if (win_check(&bo, RED, &score)) exit(0);

			int col;
			scanf("%d",&col);
			insert(&bo, col,YELLOW,&row);
			print_board(&bo);
			if (win_check(&bo, YELLOW, &score)) exit(0);
		}
	} else {

		int scores[7];

		Board *** PionsMask = (Board***)malloc(7*sizeof(Board**));
		InitMask(PionsMask);


		clock_t start, end;

		N = 0;
		struct timeval start_i;
		gettimeofday(&start_i,NULL);
		start = clock();
		int coup = cout_coup(&bo, RED, scores,PionsMask);
		end = clock();

		struct timeval end_i;
		gettimeofday(&end_i,NULL);

		double time = (end - start) / (double)CLOCKS_PER_SEC;

		printf("Wall time : %d,%ds\n",end_i.tv_sec - start_i.tv_sec);
		printf("calculated %lu nodes in %fs (%e nodes/s)\n",N,time,N/time);

		printf("%d\n",coup);

	}
}