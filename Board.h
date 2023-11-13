#include <stdint.h>


#define EMPTY 0
#define RED 1
#define YELLOW 2

typedef struct {
	uint64_t a;
	uint64_t b;
	int nb_pions;
	uint64_t zobrist_hash;
} Board;


void set_val(Board *bo, int col, int row, uint64_t val);
int get_val(Board *bo, int col, int row);
void print_board(Board *bo);