#include <stdint.h>


#define EMPTY 0
#define RED 1
#define YELLOW 2

// #define SYM_HASH

typedef struct {
	uint64_t a;
	uint64_t b;
	int nb_pions;
	uint64_t zobrist_hash;
#ifdef SYM_HASH
	uint64_t sym_zobrist_hash;
#endif
} Board;


void set_val(Board *bo, int col, int row, uint64_t val);
int get_val(Board *bo, int col, int row);
void print_board(Board *bo);