#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <math.h>

// globally white -> 0 and black -> 1
enum { white, black };

// board ranks inverted so that a1 gets 0 and b1 gets 1 and so on ..
enum {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8 
};

/*
  ranks from enum are reversed in order so that the original board (with white at bottom) are given values as shown below

  56 57 58 59 60 61 62 63 
  48 49 50 51 52 53 54 55 
  40 41 42 43 44 45 46 47 
  32 33 34 35 36 37 38 39 
  24 25 26 27 28 29 30 31 
  16 17 18 19 20 21 22 23 
  8  9  10 11 12 13 14 15 
  0  1  2  3  4  5  6  7  
  
*/

// convert pos1D to chess notation
const char* pos1D_to_notation[] = {
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8" 
};

const int bishop_occupancy_bitcount[] = {
  6,  5,  5,  5,  5,  5,  5,  6, 
  5,  5,  5,  5,  5,  5,  5,  5, 
  5,  5,  7,  7,  7,  7,  5,  5, 
  5,  5,  7,  9,  9,  7,  5,  5, 
  5,  5,  7,  9,  9,  7,  5,  5, 
  5,  5,  7,  7,  7,  7,  5,  5, 
  5,  5,  5,  5,  5,  5,  5,  5, 
  6,  5,  5,  5,  5,  5,  5,  6 
};

const int rook_occupancy_bitcount[] = {
  12,  11,  11,  11,  11,  11,  11,  12, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  12,  11,  11,  11,  11,  11,  11,  12
};

// magic numbers for edges of the board
const uint64_t rank_1 = 255ULL;
const uint64_t rank_8 = 18374686479671623680ULL;
const uint64_t file_a = 72340172838076673ULL;
const uint64_t file_b = 144680345676153346ULL;
const uint64_t file_g = 4629771061636907072ULL;
const uint64_t file_h = 9259542123273814144ULL;

// pawn attacks_mask table [color][pos1D]
uint64_t pawn_attacks_mask[2][64];

// knight attacks_mask table [pos1D]
uint64_t knight_attacks_mask[64];

// king attacks_mask table [pos1D]
uint64_t king_attacks_mask[64];


/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
/* start of section ~ ~ ~ ~ ~ ~ */
/* important bit operations */

// get i'th bit of bitboard
static inline int get_bit(const uint64_t bitboard, const int pos1D) {
  return ((bitboard & ((uint64_t)1 << pos1D)) ? 1 : 0);
}

// set i'th bit of bitboard
static inline void set_bit(uint64_t* bitboard, const int pos1D) {
  *bitboard |= ((uint64_t)1 << pos1D);
}

// flip i'th bit of bitboard
static inline void flip_bit(uint64_t* bitboard, const int pos1D) {
  *bitboard ^= ((uint64_t)1 << pos1D);
}

// reset i'th bit of bitboard to 0
static inline void reset_bit(uint64_t* bitboard, const int pos1D) {
  *bitboard &= ~((uint64_t)1 << pos1D);
}

// count the number of set bit set

static inline int popcount(uint64_t bitboard) {
  return __builtin_popcountll(bitboard);
  /*
  if compiler doesn't support given builtin function
  
  int count = 0;

  while (bitboard) {
    bitboard &= bitboard - 1;
    ++count;
  }
  return count;
  */
}



// get LSB index
static inline int LSB_index(uint64_t bitboard) {
  if (bitboard) {
    return __builtin_ctzll(bitboard);
  }
  else {
    return -1; // error case
  }
  /*
  if compiler doesn't support given builtin function
  
  if (bitboard) {
    return popcount((bitboard & (~bitboard + 1)) - 1);
  }
  else {
    // handle edge error cases
  }
  */
}


/* end of section ~ ~ ~ ~ ~ ~ */
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */



/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
/* start of section ~ ~ ~ ~ ~ ~ */
/* visual representation */

// print bitboard
void print_bitboard(const uint64_t bitboard) {
  printf("\n");
  for (int rank = 7; rank > -1; --rank) {
    for (int file = 0; file < 8; ++file) {
      int pos1D = rank*8 + file;
      // printing ranks
      if (!file) {
        printf("    %d  ", rank + 1);
      }
      printf(" %d", get_bit(bitboard, pos1D));
    }
    printf("\n");
  }
  // printing files
  printf("\n        a b c d e f g h\n");
  printf("\n    Bitboard value: %llu", bitboard);
  printf("\n\n");
}

/* end of section ~ ~ ~ ~ ~ ~ */
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */



/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
/* start of section ~ ~ ~ ~ ~ ~ */
/* attacks_mask */

// get pawn attacks_mask
uint64_t mask_pawn_attacks_mask(const int color, const int pos1D) {
  
  // result attacks_mask 
  uint64_t attacks_mask = 0ULL;
  
  // set piece on board
  uint64_t bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
// white pawns
  if (!color) {
    if (bitboard & ~file_h) {
      attacks_mask |= (bitboard << 9);
    }
    if (bitboard & ~file_a) {
      attacks_mask |= (bitboard << 7);
    }
    
  }
  // black pawns
  else {
    if (bitboard & ~file_h) {
      attacks_mask |= (bitboard >> 7);
    }
    if (bitboard & ~file_a) {
      attacks_mask |= (bitboard >> 9);
    }
  }

  return attacks_mask;
}

// pre-compute pawn attacks_mask  
void precompute_pawn_attacks_mask(uint64_t pawn_attacks_mask[2][64]) {
  for (int pos1D = 0; pos1D < 64; ++pos1D) {
    pawn_attacks_mask[white][pos1D] = mask_pawn_attacks_mask(white,pos1D);
    pawn_attacks_mask[black][pos1D] = mask_pawn_attacks_mask(black,pos1D);
  }
}

// get knight attacks_mask 
uint64_t mask_knight_attacks_mask(const int pos1D) {
  // result attacks_mask 
  uint64_t attacks_mask = 0ULL;
  
  // set piece on board
  uint64_t bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
  // 2 up + 1 left
  if ((bitboard << 15) & ~file_h) attacks_mask |= (bitboard << 15);
  // 2 up + 1 right
  if ((bitboard << 17) & ~file_a) attacks_mask |= (bitboard << 17);
  // 2 down + 1 left
  if((bitboard >> 17) & ~file_h) attacks_mask |= (bitboard >> 17);
  // 2 down + 1 right
  if ((bitboard >> 15) & ~file_a) attacks_mask |= (bitboard >> 15);
  // 2 left + 1 up
  if ((bitboard << 6) & ~(file_h | file_g)) attacks_mask |= (bitboard << 6);
  // 2 left + 1 down
  if ((bitboard >> 10) & ~(file_h | file_g)) attacks_mask |= (bitboard >> 10);
  // 2 right + 1 up
  if ((bitboard << 10) & ~(file_a | file_b)) attacks_mask |= (bitboard << 10);
  // 2 right + 1 down
  if ((bitboard >> 6) & ~(file_a | file_b)) attacks_mask |= (bitboard >> 6);

  return attacks_mask;
}

// pre-compute knight attacks_mask  
void precompute_knight_attacks_mask(uint64_t knight_attacks_mask[64]) {
  for (int pos1D = 0; pos1D < 64; ++pos1D) {
    knight_attacks_mask[pos1D] = mask_knight_attacks_mask(pos1D);
  }
}

// get king attacks_mask
uint64_t mask_king_attacks_mask(const int pos1D) {
  // result attacks_mask
  uint64_t attacks_mask = 0ULL;
  
  // set piece on board
  uint64_t bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
  // 1 up
  if (bitboard << 8) attacks_mask |= (bitboard << 8);
  // 1 up + 1 left
  if ((bitboard << 7) & ~file_h) attacks_mask |= (bitboard << 7);
  // 1 up + 1 right
  if ((bitboard << 9) & ~file_a) attacks_mask |= (bitboard << 9);
  // 1 right
  if ((bitboard << 1) & ~file_a) attacks_mask |= (bitboard << 1);

  // 1 down
  if (bitboard >> 8) attacks_mask |= (bitboard >> 8);
  // 1 down + 1 left
  if ((bitboard >> 9) & ~file_h) attacks_mask |= (bitboard >> 9);
  // 1 down + 1 right
  if ((bitboard >> 7) & ~file_a) attacks_mask |= (bitboard >> 7);
  // 1 left
  if ((bitboard >> 1) & ~file_h) attacks_mask |= (bitboard >> 1);
  
  return attacks_mask;
}

// pre-compute king attacks_mask 
void precompute_king_attacks_mask(uint64_t king_attacks_mask[64]) {
  for (int pos1D = 0; pos1D < 64; ++pos1D) {
    king_attacks_mask [pos1D] = mask_king_attacks_mask (pos1D);
  }
}

// get bishop occupancy mask
uint64_t mask_bishop_occupancy(const int pos1D) {
  // result occupancy mask
  uint64_t occupancy = 0ULL;

  // bishop position
  int rank, file;
  // occupancy position
  int occupancy_rank, occupancy_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // diagonals
  // up + right
  for (occupancy_rank = rank +  1, occupancy_file = file + 1; occupancy_rank < 7 && occupancy_file < 7; ++occupancy_rank, ++occupancy_file) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }
  // up + left
  for (occupancy_rank = rank +  1, occupancy_file = file - 1; occupancy_rank < 7 && occupancy_file > 0; ++occupancy_rank, --occupancy_file) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }
  // down + right
  for (occupancy_rank = rank -  1, occupancy_file = file + 1; occupancy_rank > 0 && occupancy_file < 7; --occupancy_rank, ++occupancy_file) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }
  // down + left
  for (occupancy_rank = rank -  1, occupancy_file = file - 1; occupancy_rank > 0 && occupancy_file > 0; --occupancy_rank, --occupancy_file) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }

  return occupancy;
}

// get bishop attacks_mask given occupancy mask
uint64_t mask_bishop_attacks_mask_given_occupancy(const int pos1D, const uint64_t occupancy) {
  // result attacks_mask  
  uint64_t attacks_mask = 0ULL;

  // bishop position
  int rank, file;
  // attacks_mask position
  int attacks_mask_rank, attacks_mask_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // diagonals
  // up + right
  for (attacks_mask_rank = rank +  1, attacks_mask_file = file + 1; attacks_mask_rank < 8 && attacks_mask_file < 8; ++attacks_mask_rank, ++attacks_mask_file) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }
  // up + left
  for (attacks_mask_rank = rank +  1, attacks_mask_file = file - 1; attacks_mask_rank < 8 && attacks_mask_file > -1; ++attacks_mask_rank, --attacks_mask_file) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }
  // down + right
  for (attacks_mask_rank = rank -  1, attacks_mask_file = file + 1; attacks_mask_rank > -1 && attacks_mask_file < 8; --attacks_mask_rank, ++attacks_mask_file) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }
  // down + left
  for (attacks_mask_rank = rank -  1, attacks_mask_file = file - 1; attacks_mask_rank > -1 && attacks_mask_file > -1; --attacks_mask_rank, --attacks_mask_file) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }

  return attacks_mask;
}

// get rook occupancy mask
uint64_t mask_rook_occupancy(const int pos1D) {

  // result occupancy mask 
  uint64_t occupancy = 0ULL;

  // rook position
  int rank, file;
  // occupancy position
  int occupancy_rank, occupancy_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // straight lines
  // up
  for (occupancy_rank = rank +  1 , occupancy_file = file; occupancy_rank < 7; ++occupancy_rank) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }
  // right
  for (occupancy_rank = rank , occupancy_file = file + 1;occupancy_file < 7; ++occupancy_file) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }
  // down
  for (occupancy_rank = rank -  1 , occupancy_file = file; occupancy_rank > 0; --occupancy_rank) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }
  // left
  for (occupancy_rank = rank , occupancy_file = file - 1 ; occupancy_file > 0; --occupancy_file) {
    occupancy |= ((uint64_t)1 << (occupancy_rank * 8 + occupancy_file));
  }

  return occupancy;
}

// get rook attacks_mask given occupancy mask
uint64_t mask_rook_attacks_mask_given_occupancy(const int pos1D, const uint64_t occupancy) {

  // result attacks_mask 
  uint64_t attacks_mask = 0ULL;

  // rook position
  int rank, file;
  // attacks_mask position
  int attacks_mask_rank, attacks_mask_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // straight lines
  // up
  for (attacks_mask_rank = rank +  1 , attacks_mask_file = file; attacks_mask_rank < 8; ++attacks_mask_rank) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }
  // right
  for (attacks_mask_rank = rank , attacks_mask_file = file + 1;attacks_mask_file < 8; ++attacks_mask_file) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }
  // down
  for (attacks_mask_rank = rank -  1 , attacks_mask_file = file; attacks_mask_rank > -1; --attacks_mask_rank) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }
  // left
  for (attacks_mask_rank = rank , attacks_mask_file = file - 1 ; attacks_mask_file > -1; --attacks_mask_file) {
    attacks_mask |= ((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file));
    if (((uint64_t)1 << (attacks_mask_rank * 8 + attacks_mask_file)) & occupancy) {
      break;
    }
  }

  return attacks_mask;
}

// generates ith combination from all possible occupancy
uint64_t ith_occupancy_combination(const int ith_combination, const int bits_in_mask, uint64_t occupancy_mask) {
  // ith combination ranges from 0 .. 2^(bits in mask) - 1

  // result ith occupancy mask
  uint64_t ith_occupancy_mask = 0ULL;

  for (int i = 0; i < bits_in_mask; ++i) {
    // LSB index
    int pos1D = LSB_index(occupancy_mask);

    // remove LSB
    reset_bit(&occupancy_mask, pos1D);

    // add occupancy if its in ith combination
    if (ith_combination & (1 << i)) {
      ith_occupancy_mask |= ((uint64_t)1 << pos1D);
    }
  }

  return ith_occupancy_mask;
}

uint32_t random_U32_number() {
  return random();

  /*
  //if random() not supported (since it is only supported on POSIX systems)
  
  //use, XORSHIFT32 algorithm
  
  //initialize a global unsigned int variable(named state) to a random 32 bit number(non zero) first
  unsigned int number = state;
  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;
  state = number;
  return number;
  */
  
}

uint64_t random_U64_number() {
    uint64_t u1, u2, u3, u4;
    u1 = (uint64_t)(random()) & 0xFFFF;
    u2 = (uint64_t)(random()) & 0xFFFF;
    u3 = (uint64_t)(random()) & 0xFFFF;
    u4 = (uint64_t)(random()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

// sparsely populated 64 bit number
uint64_t random_U64_number_low_population() {
  return random_U64_number() & random_U64_number() & random_U64_number();
}

uint64_t magic_number(int pos1D, int piece_occupancy_bitcount, uint64_t attack) {
  
}

void precompute_piece_attacks_mask() {

}

/* end of section ~ ~ ~ ~ ~ ~ */
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

int main() {
  uint64_t bitboard = 0ULL;

  for (int i = 0; i < 10; ++i){
    print_bitboard(random_U64_number_low_population());
  }
  
  
	return 0;
}
