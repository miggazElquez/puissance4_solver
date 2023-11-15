#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <getopt.h>

#include "Board.h"

// Optimization flags
#define MULTITHREADING 	0
#define USE_HASHMAP 	1

//Hash map
//different types of hash function
#define BASIC_XOR 		1
#define XOR_ADD 		2
#define XOR_32 			3
#define XOR_ADD_32 		4
#define XOR_16 			5
#define XOR_ADD_16 		6
#define ZOBRIST_HASH 	7

#define HASH_FUNCTION ZOBRIST_HASH //Use one of the defined Hash


#define MOST_RECENT 1
#define LOWER_DEPTH 2

#define REPLACEMENT_STRAT LOWER_DEPTH

/* Global parameters */
static uint8_t maxDepth = 11;
static uint8_t isInteractive = 1;
static uint8_t csvOutput = 0;
static uint32_t hmapSize = 65536;
static uint32_t hmapSizeMask = 65536 - 1;

typedef struct {
   Board bo;
   int value;
   int cut;
} HashMap_Val;

uint64_t ZOBRIST_RANDOM[84];

////////// ALTERNATIVE avec masques //////////
int insert(Board *bo, int col, int color,int* rowrec ) {
	/*if (col > 6) {
		printf("col trop grande\n");
		return 1;
	}*/
	int row;
	for (row =0;row<6;row++) {
		if (get_val(bo,col,row) == EMPTY) {
			set_val(bo,col,row,color);
			*rowrec = row;
			bo->nb_pions++;
			if (USE_HASHMAP == 1 && HASH_FUNCTION == ZOBRIST_HASH) {
				int index = (col*6 + row) * color;
				bo->zobrist_hash ^= (ZOBRIST_RANDOM[index]);
				#ifdef SYM_HASH
				int index2 = ((6-col)*6 + row) * color;				
				bo->sym_zobrist_hash ^= (ZOBRIST_RANDOM[index2]);
				#endif
			}
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

int win_check2(Board *bo, int color,int col,int row, int *score, Board ** PionsMask) {
	int res = color==RED?1:-1;
	int draw = 1;
	int p=0;
	
	
	Board TestBoard;
	if(bo->nb_pions+1==42){
		*score = 0;
		return 1;
	}
	int colrow = col*6+row;
	if(color==RED){
		while((PionsMask[colrow][p].a>0) || (PionsMask[colrow][p].b>0)){
			TestBoard.a=PionsMask[colrow][p].a & bo->a;
			TestBoard.b=PionsMask[colrow][p].b & bo->b;
			if((TestBoard.a==PionsMask[colrow][p].a)&&(TestBoard.b==PionsMask[colrow][p].b)){
				*score = res;
				return 1;
			}
			p++;
		}
	} else {
		while((PionsMask[colrow][p].a>0) || (PionsMask[colrow][p].b>0)){
			TestBoard.a=(PionsMask[colrow][p].a<<1) & bo->a;
			TestBoard.b=(PionsMask[colrow][p].b<<1) & bo->b;
			if((TestBoard.a==(PionsMask[colrow][p].a<<1))&&(TestBoard.b==(PionsMask[colrow][p].b<<1))){
				*score = res;
				return 1;
			}
			p++;
		}
	}

	return 0;
}


uint64_t hash_basic_xor(Board *bo) {
	return bo->a ^ bo->b;
}

uint64_t hash_xor_add(Board *bo) {
	return bo->a ^ bo->b + bo->nb_pions;
}


uint64_t hash_xor_32(Board *bo) {
	return (bo->a & 0xffffffff) ^ (bo->a >> 32) ^ (bo->b & 0xffffffff) ^ (bo->b >> 32);
}

uint64_t hash_xor_32_add(Board *bo) {
	return (bo->a & 0xffffffff) ^ (bo->a >> 32) ^ (bo->b & 0xffffffff) ^ (bo->b >> 32) + bo->nb_pions;
}

uint64_t hash_xor_16(Board *bo) {
	uint64_t a = ((bo->a >> 32) & 0xFFFF) ^ ((bo->a >> 16) & 0xFFFF) ^ (bo->a & 0xFFFF);
	uint64_t b = ((bo->b >> 32) & 0xFFFF) ^ ((bo->b >> 16) & 0xFFFF) ^ (bo->b & 0xFFFF);
	return a ^ b;
}

uint64_t hash_xor_16_add(Board *bo) {
	//We don't need all 16bits part : a fits on 48 bits, and b on 36 bits
	//
	uint64_t a = ((bo->a >> 32) & 0xFFFF) ^ ((bo->a >> 16) & 0xFFFF) ^ (bo->a & 0xFFFF);
	uint64_t b = ((bo->b >> 32) & 0xFFFF) ^ ((bo->b >> 16) & 0xFFFF) ^ (bo->b & 0xFFFF);

	return a ^ b + bo->nb_pions;
}

uint64_t hash_zobrist(Board *bo) {
	return bo->zobrist_hash;
}


void compute_sym(uint64_t* a_, uint64_t* b_) {
	uint64_t mask = 0xfff;
	uint64_t temp;

    uint64_t a = *a_;
    uint64_t b = *b_;
	
	temp = a & mask;
	a &= ~mask;
	a |= (b >> 24) & mask;
	b &= ~(mask << 24);
	b |= temp << 24; 

	temp = (a >> 12) & mask;
	a &= ~(mask << 12);
	a |= b & (mask << 12);
	b &= ~(mask << 12);
	b |= temp << 12;

	temp = (a >> 24) & mask;
	a &= ~(mask << 24);
	a |= (b & mask) << 24;
	b &= ~mask;
	b |= temp;

    *a_ = a;
    *b_ = b;

}


//From https://stackoverflow.com/questions/33010010/how-to-generate-random-64-bit-unsigned-integer-in-c
uint64_t rand_uint64(void) {
  uint64_t r = 0;
  for (int i=0; i<64; i += 15 /*30*/) {
    r = r*((uint64_t)RAND_MAX + 1) + rand();
  }
  return r;
}


void init_zobrist() {
	srand(28600492);
	for (int i=0;i<84;i++) {
		ZOBRIST_RANDOM[i] = rand_uint64();
	}
}

uint64_t hash_board(Board *bo) {
	if (HASH_FUNCTION == BASIC_XOR) {
		return hash_basic_xor(bo);
	} else if (HASH_FUNCTION == XOR_ADD) {
		return hash_xor_add(bo);
	} else if (HASH_FUNCTION == XOR_32) {
		return hash_xor_32(bo);
	} else if (HASH_FUNCTION == XOR_ADD_32) {
		return hash_xor_32_add(bo);
	} else if (HASH_FUNCTION == XOR_16) {
		return hash_xor_16(bo);
	} else if (HASH_FUNCTION == XOR_ADD_16) {
		return hash_xor_16_add(bo);
	} else if (HASH_FUNCTION == ZOBRIST_HASH) {
		return hash_zobrist(bo);
	} else {
		printf("No hash function defined\n");
		exit(0);
	}
}



///////////////

volatile uint64_t NODES = 0;
uint64_t HIT = 0;
uint64_t MISS = 0;
uint64_t SYM_HIT = 0;



//////////////////////////

int max(Board *bo,int depth,int alpha,Board ** PionsMask,const int colR,const int row,HashMap_Val *hash_map);
int min(Board *bo,int depth,int beta,Board ** PionsMask,const int colR,const int row,HashMap_Val *hash_map) { //Yellow to play
	int score = 2;
	// printf("min, depth : %d\n",depth);
	// print_board(bo);

	NODES++;
	
	if (win_check2(bo,RED,colR,row,&score,PionsMask)) {
		return score;
	}

	int cutoff = 0;

	for (int col=0;col<7;col++) {
		Board temp = *bo;
		int rowtemp;
		if (insert(&temp,col,YELLOW,&rowtemp)) continue;

		int val;
		if (USE_HASHMAP == 1) {
			uint64_t hash = hash_board(&temp) & hmapSizeMask;
			HashMap_Val h = hash_map[hash];

			if (h.bo.a == temp.a && h.bo.b == temp.b) {
				if (h.cut) {
					if (h.value >= score) {
						HIT++;
						val = h.value;
					} else {
						val = max(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
						MISS++;
					}
				} else {
					val = h.value;
					HIT++;
				}
			} else {
				#ifdef SYM_HASH
				uint64_t hash2 = temp.sym_zobrist_hash & hmapSizeMask;
				HashMap_Val h2 = hash_map[hash2];
				
				uint64_t a=h2.bo.a, b=h2.bo.b;
				compute_sym(&a,&b);

				if (a == temp.a && b == temp.b) {
					SYM_HIT++;
					if (h2.cut) {
						if (h2.value >= score) {
							HIT++;
							val = h2.value;
						} else {
							val = max(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
							MISS++;
							SYM_HIT--;
						}
					} else {
						val = h.value;
						HIT++;
					}


				} else {
				#endif

				val = max(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
				MISS++;

				#ifdef SYM_HASH
				}
				#endif
			}
		} else {
			val = max(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
		}

		if (val < score) {
			score = val;
			if (score == -1) {
				goto end_function;
			}
			if (score <= beta) {
				cutoff = 1;
				goto end_function;
			}
		}
	}

	// printf("-> min, depth : %d, score : %d\n",depth,score);
end_function:
	if (USE_HASHMAP == 1) {
		uint64_t hash = hash_board(bo) & hmapSizeMask;
		int replace = 0;
		if (REPLACEMENT_STRAT == MOST_RECENT) {
			replace = 1;
		} else if (REPLACEMENT_STRAT == LOWER_DEPTH) {
			HashMap_Val h = hash_map[hash];
			if (h.bo.a != 0 || h.bo.b != 0) {
				if (bo->nb_pions <= h.bo.nb_pions) { //current node is higher in the tree
					replace = 1;
				} else {
					replace = 0;
				}
			} else {
				replace = 1;
			}
		}

		if (replace) {
			hash_map[hash].bo = *bo;
			hash_map[hash].value = score;
			hash_map[hash].cut = cutoff;
		}

	}
	return score;

}

int max(Board *bo,int depth,int alpha,Board ** PionsMask,const int colR,const int row,HashMap_Val *hash_map) { //Red to play
	// printf("max, depth : %d\n",depth);
	// print_board(bo);

	NODES++;

	if (depth > maxDepth) return 0;
	int score = -2;

	if (win_check2(bo,YELLOW,colR,row,&score,PionsMask)) {
		return score;
	}

	int cutoff = 0;
	for (int col=0;col<7;col++) {
		Board temp = *bo;
		int rowtemp;
		if (insert(&temp,col,RED,&rowtemp)) continue;

		int val;
		if (USE_HASHMAP == 1) {
			uint64_t hash = hash_board(&temp) & hmapSizeMask;
			HashMap_Val h = hash_map[hash];
			if (h.bo.a == temp.a && h.bo.b == temp.b) {
				if (h.cut) {
					if (h.value <= score) {
						HIT++;
						val = h.value; //voir `continue` ?
					} else {
						val = min(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
						MISS++;
					}
				} else {
					val = h.value;
					HIT++;
				}
			} else {
				#ifdef SYM_HASH
				uint64_t hash2 = temp.sym_zobrist_hash & hmapSizeMask;
				HashMap_Val h2 = hash_map[hash2];
				
				uint64_t a=h2.bo.a, b=h2.bo.b;
				compute_sym(&a,&b);
				if (a == temp.a && b == temp.b) {
					SYM_HIT++;
					if (h2.cut) {
						if (h2.value <= score) {
							HIT++;
							val = h2.value;
						} else {
							val = min(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
							MISS++;
							SYM_HIT--;
						}
					} else {
						val = h.value;
						HIT++;
					}


				} else {
				#endif

				val = min(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
				MISS++;

				#ifdef SYM_HASH
				}
				#endif
			}
		} else {
			val = min(&temp,depth+1,score,PionsMask,col,rowtemp,hash_map);
		}

		if (val > score) {
			score = val;
			if (score == 1)
				goto end_function;

			if (score >= alpha){
				cutoff = 1;
				goto end_function;
			}

		}
	}

end_function:
	if (USE_HASHMAP == 1) {
		uint64_t hash = hash_board(bo) & hmapSizeMask;
		int replace = 0;
		if (REPLACEMENT_STRAT == MOST_RECENT) {
			replace = 1;
		} else if (REPLACEMENT_STRAT == LOWER_DEPTH) {
			HashMap_Val h = hash_map[hash];
			if (h.bo.a != 0 || h.bo.b != 0) {
				if (bo->nb_pions <= h.bo.nb_pions) { //current node is higher in the tree
					replace = 1;
				} else {
					replace = 0;
				}
			}
		}

		if (replace) {
			hash_map[hash].bo = *bo;
			hash_map[hash].value = score;
			hash_map[hash].cut = cutoff;
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
	Board ** PionsMask;
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
		val = min(&temp,0,-2,a->PionsMask,a->i,row,NULL); //-2 to not have alpha beta pruning
	} else {
		val = max(&temp,0,2,a->PionsMask,a->i,row,NULL); //same
	}

	if (!csvOutput)
		printf("	%d : %d\n",a->i,val);
	a->res[a->i] = val;

}


int cout_coup(Board *bo,int current_color, int* res,Board ** PionsMask,HashMap_Val *hash_map) {
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

				int val;
				if (USE_HASHMAP == 1) {
					#ifdef SYM_HASH
					uint64_t hash2 = temp.sym_zobrist_hash & hmapSizeMask;
					HashMap_Val h2 = hash_map[hash2];
					uint64_t a=h2.bo.a, b=h2.bo.b;
					compute_sym(&a,&b);
					if (a == temp.a && b == temp.b) {
						SYM_HIT++;
						if (h2.cut) {
							val = min(&temp,0,-2,PionsMask,col,rowtemp,hash_map);
							MISS++;
							SYM_HIT--;
						} else {
							val = h2.value;
							HIT++;
						}
					} else {
						val = min(&temp,0,-2,PionsMask,col,rowtemp,hash_map);
					}
					#endif

					#ifndef SYM_HASH
						val = min(&temp,0,-2,PionsMask,col,rowtemp,hash_map);
					#endif
				} else {
					val = min(&temp,0,-2,PionsMask,col,rowtemp,hash_map);
				}

				if (!csvOutput)
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
			int depth = 0;
			for (int col=0;col<7;col++) {
				Board temp = *bo;
				int rowtemp;
				if (insert(&temp,col,YELLOW,&rowtemp)) continue;

				int val;
				if (USE_HASHMAP == 1) {
					#ifdef SYM_HASH
					uint64_t hash2 = temp.sym_zobrist_hash & hmapSizeMask;
					HashMap_Val h2 = hash_map[hash2];
					
					uint64_t a=h2.bo.a, b=h2.bo.b;
					compute_sym(&a,&b);
					if (a == temp.a && b == temp.b) {
						SYM_HIT++;
						if (h2.cut) {
							val = max(&temp,0,2,PionsMask,col,rowtemp,hash_map);
							MISS++;
							SYM_HIT--;
						} else {
							printf("SYM HIT, col=%d\n",col);
							val = h2.value;
							HIT++;
						}
					} else {
						val = max(&temp,0,2,PionsMask,col,rowtemp,hash_map);
					}
					#endif

					#ifndef SYM_HASH
					val = max(&temp,0,2,PionsMask,col,rowtemp,hash_map); //They will not be any result in the hashmap
					#endif
				} else {
					val = max(&temp,0,2,PionsMask,col,rowtemp,hash_map);
				}
				
				if (!csvOutput)
					printf("	%d : %d\n",col,val);
				if (val > score) {
					score = val;
					coup = col;
				}
			}
			*res = score;
			return coup;

		}

	}
}




int InitMask(Board ** PionsMask){
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
		
		
	if (!csvOutput)
		printf("Masque crées\n");

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
			//Pour les verticaux ne pas mettre les masques aux premieres ligne 0,1,2 et faire attenttion au ligne 3,4
			if(row > 2){
				TempMask[cptNbmask]=boV[row-3][col];
				cptNbmask++;
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
			int colrow = col*6 +row;
			//fin de l'explo de tous les masques 
			PionsMask[colrow]=(Board*)malloc((cptNbmask+1)*sizeof(Board));
			memset(PionsMask[colrow],0,(cptNbmask+1)*sizeof(Board));
			memcpy(PionsMask[colrow],TempMask,(cptNbmask+1)*sizeof(Board));
			memset(TempMask,0,14*sizeof(Board));
			
		}
	}
	return 1;
}



int main(int argc, char *argv[]) {
	extern char *optarg;
	int c;

	while ((c = getopt(argc, argv, "sfd:h:")) != -1) {
		switch (c) {
			case 'f': isInteractive = 0; break;
			case 's': csvOutput = 1; break;
			case 'h': {
				uint8_t tmp_hms = strtoul(optarg, NULL, 10);
				if (tmp_hms) {
					hmapSize = 1 << tmp_hms;
					hmapSizeMask = hmapSize - 1;
				}
				break;
			}
			case 'd': {
				uint8_t tmp_depth = strtoul(optarg, NULL, 10);
				if (tmp_depth)
					maxDepth = tmp_depth;
				break;	
			}
			default:
				break;
		}
	}

	uint64_t val = 0;
	Board bo = {
		.a = 0,
		.b = 0,
		.nb_pions = 0,
		.zobrist_hash = 0
	};

	int current_color = RED;
	if (!csvOutput || isInteractive) {
		print_board(&bo);
	}

	if (isInteractive) {
		int col;
		Board ** PionsMask = (Board**)malloc(7*6*sizeof(Board*));
		InitMask(PionsMask);

		HashMap_Val *hash_map;

		if (USE_HASHMAP == 1) {
			hash_map = malloc(hmapSize * sizeof(HashMap_Val));
			memset(hash_map,0,hmapSize * sizeof(HashMap_Val));
			if (HASH_FUNCTION == ZOBRIST_HASH) {
				init_zobrist();
			}
		}

		while (1) {
			int scores[7];
			int score;
			
			int row;
			
			clock_t start, end;
			
			NODES = 0;
			struct timeval start_i;
			gettimeofday(&start_i,NULL);
			printf("Lancement ...\n");
			start = clock();
			
			if (USE_HASHMAP == 1) {
				memset(hash_map,0,hmapSize * sizeof(HashMap_Val));
				HIT = 0;
				MISS = 0;
				SYM_HIT = 0;
			}
			int coup = cout_coup(&bo, RED, &score,PionsMask,hash_map);

			end = clock();

			struct timeval end_i;
			gettimeofday(&end_i,NULL);

			double time = (end - start) / (double)CLOCKS_PER_SEC;

			printf("Wall time : %ds\n",end_i.tv_sec - start_i.tv_sec);
			printf("calculated %lu nodes in %fs (%e nodes/s)\n", NODES, time, NODES/time);
			printf("%d HIT, %f%\n",HIT, HIT / (float)(HIT+MISS) * 100);
			#ifdef SYM_HASH
			printf("%d SYM_HIT,%f%\n",SYM_HIT,SYM_HIT / (float)(HIT+MISS) * 100);
			#endif
			if (USE_HASHMAP == 1) {
				int count = 0;
				for (int i=0;i<hmapSize;i++) {
					if (hash_map[i].bo.a != 0 || hash_map[i].bo.b != 0) {
						count++;
					}
				}
				printf("count : %d (%f%)\n",count, (float)count / hmapSize * 100);
			}

			printf("played %d\n",coup);
			printf("nb_pions:%d\n",bo.nb_pions);
			insert(&bo, coup,RED,&row);

			print_board(&bo);
			
			if (win_check(&bo, RED, &score)) {
				printf("FIN DE PARTIE\n");
				printf("%lx %lx\n",bo.a, bo.b);
				exit(0);
			}
			int col;
			int played = 1;
			while (played) {
				printf("Your move : ");
				scanf("%d",&col);
				played = insert(&bo, col,YELLOW,&row);
			}
			print_board(&bo);
			if (win_check(&bo, YELLOW, &score)) {
				printf("FIN DE PARTIE\n");
				printf("%lx %lx\n",bo.a, bo.b);
				exit(0);
			}
		}
	} else {
		int scores[7];

		Board ** PionsMask = (Board**)malloc(7*6*sizeof(Board*));
		InitMask(PionsMask);

		HashMap_Val *hash_map;

		if (USE_HASHMAP == 1) {
			hash_map = malloc(hmapSize * sizeof(HashMap_Val));
			memset(hash_map,0,hmapSize * sizeof(HashMap_Val));
			if (HASH_FUNCTION == ZOBRIST_HASH) {
				init_zobrist();
			}

		}

		clock_t start, end;

		NODES = 0;

		struct timeval start_i;
		gettimeofday(&start_i,NULL);
		start = clock();

		int coup = cout_coup(&bo, RED, scores, PionsMask, hash_map);

		end = clock();
		struct timeval end_i;
		gettimeofday(&end_i,NULL);

		double time = (end - start) / (double)CLOCKS_PER_SEC;
		int wall_time = end_i.tv_sec - start_i.tv_sec;
		double nodes_p_s = NODES / (double) time;
		double hit_rate = HIT / (double) (HIT + MISS);
		double sym_hit_rate = SYM_HIT / (double) (HIT + MISS);


		if (csvOutput) {
			printf("wall_time,nodes_calculated,time,nodes_p_s,hits,hit_rate,sym_hit_rate\n");
			printf("%d,%lu,%f,%e,%d,%f,%f\n", wall_time, NODES, time, nodes_p_s, HIT, hit_rate,sym_hit_rate);
		} else {
			printf("Wall time : %ds\n", wall_time);
			printf("calculated %lu nodes in %fs (%e nodes/s)\n", NODES, time, nodes_p_s);
			printf("%d HIT, %f%\n", HIT, hit_rate * 100);
			#ifdef SYM_HASH
			printf("%d SYM_HIT,%f%\n",SYM_HIT,SYM_HIT / (float)(HIT+MISS) * 100);
			#endif

		}

		if (USE_HASHMAP == 1) {
			int count = 0;
			for (int i=0;i<hmapSize;i++) {
				if (hash_map[i].bo.a != 0 || hash_map[i].bo.b != 0) {
					count++;
				}
			}
			if (!csvOutput)
				printf("count : %d (%f%)\n",count, (float)count / hmapSize * 100);
		}

		if (!csvOutput)
			printf("%d\n",coup);
	}
}
