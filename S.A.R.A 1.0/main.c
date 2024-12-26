#include <stdio.h>

// globally white -> 0 and black -> 1
enum { white, black };

// useful for debugging
enum{
  // board ranks inverted so that a1 gets 0 and b1 gets 1 and so on ..
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

// magic numbers for edges of the board
const unsigned long long rank_1 = 255ULL;
const unsigned long long rank_8 = 18374686479671623680ULL;
const unsigned long long file_a = 72340172838076673ULL;
const unsigned long long file_b = 144680345676153346ULL;
const unsigned long long file_g = 4629771061636907072ULL;
const unsigned long long file_h = 9259542123273814144ULL;

// pawn attacks table [color][pos1D]
unsigned long long pawn_attacks[2][64];

// knight attacks table [pos1D]
unsigned long long knight_attacks[64];

// king attacks table [pos1D]
unsigned long long king_attacks[64];


/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
/* start of section ~ ~ ~ ~ ~ ~ */
/* important bit operations */

// get i'th bit of bitboard
int get_bit(const unsigned long long bitboard, const int pos1D){
  return ((bitboard & (1ULL << pos1D)) ? 1 : 0);
}

// set i'th bit of bitboard
void set_bit(unsigned long long* bitboard, const int pos1D){
  *bitboard |= (1ULL << pos1D);
}

// flip i'th bit of bitboard
void flip_bit(unsigned long long* bitboard, const int pos1D){
  *bitboard ^= (1ULL << pos1D);
}

// reset i'th bit of bitboard to 0
void reset_bit(unsigned long long* bitboard, const int pos1D){
  *bitboard &= ~(1ULL << pos1D);
}

/* end of section ~ ~ ~ ~ ~ ~ */
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */



/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
/* start of section ~ ~ ~ ~ ~ ~ */
/* visual representation */

// print bitboard
void print_bitboard(const unsigned long long bitboard){
  printf("\n");
  for (int rank = 7; rank > -1; --rank){
    for (int file = 0; file < 8; ++file){
      int pos1D = rank*8 + file;
      // printing ranks
      if (!file){
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
/* attacks */

// get pawn attacks mask
unsigned long long mask_pawn_attacks(const int color, const int pos1D){
  
  // result attacks mask
  unsigned long long attacks = 0ULL;
  
  // set piece on board
  unsigned long long bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
// white pawns
  if (!color){
    if (bitboard & ~file_h){
      attacks |= (bitboard << 9);
    }
    if (bitboard & ~file_a){
      attacks |= (bitboard << 7);
    }
    
  }
  // black pawns
  else{
    if (bitboard & ~file_h){
      attacks |= (bitboard >> 7);
    }
    if (bitboard & ~file_a){
      attacks |= (bitboard >> 9);
    }
  }

  return attacks;
}

// pre-compute pawn attacks
void precompute_pawn_attacks(unsigned long long pawn_attacks[2][64]){
  for (int pos1D = 0; pos1D < 64; ++pos1D){
    pawn_attacks[white][pos1D] = mask_pawn_attacks(white,pos1D);
    pawn_attacks[black][pos1D] = mask_pawn_attacks(black,pos1D);
  }
}

// get knight attacks mask
unsigned long long mask_knight_attacks(const int pos1D){
  // result attacks mask
  unsigned long long attacks = 0ULL;
  
  // set piece on board
  unsigned long long bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
  // 2 up + 1 left
  if ((bitboard << 15) & ~file_h) attacks |= (bitboard << 15);
  // 2 up + 1 right
  if ((bitboard << 17) & ~file_a) attacks |= (bitboard << 17);
  // 2 down + 1 left
  if((bitboard >> 17) & ~file_h) attacks |= (bitboard >> 17);
  // 2 down + 1 right
  if ((bitboard >> 15) & ~file_a) attacks |= (bitboard >> 15);
  // 2 left + 1 up
  if ((bitboard << 6) & ~(file_h | file_g)) attacks |= (bitboard << 6);
  // 2 left + 1 down
  if ((bitboard >> 10) & ~(file_h | file_g)) attacks |= (bitboard >> 10);
  // 2 right + 1 up
  if ((bitboard << 10) & ~(file_a | file_b)) attacks |= (bitboard << 10);
  // 2 right + 1 down
  if ((bitboard >> 6) & ~(file_a | file_b)) attacks |= (bitboard >> 6);

  return attacks;
}

// pre-compute knight attacks
void precompute_knight_attacks(unsigned long long knight_attacks[64]){
  for (int pos1D = 0; pos1D < 64; ++pos1D){
    knight_attacks[pos1D] = mask_knight_attacks(pos1D);
  }
}

// get king attacks mask
unsigned long long mask_king_attacks(const int pos1D){
  // result attacks mask
  unsigned long long attacks = 0ULL;
  
  // set piece on board
  unsigned long long bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
  // 1 up
  if (bitboard << 8) attacks |= (bitboard << 8);
  // 1 up + 1 left
  if ((bitboard << 7) & ~file_h) attacks |= (bitboard << 7);
  // 1 up + 1 right
  if ((bitboard << 9) & ~file_a) attacks |= (bitboard << 9);
  // 1 right
  if ((bitboard << 1) & ~file_a) attacks |= (bitboard << 1);

  // 1 down
  if (bitboard >> 8) attacks |= (bitboard >> 8);
  // 1 down + 1 left
  if ((bitboard >> 9) & ~file_h) attacks |= (bitboard >> 9);
  // 1 down + 1 right
  if ((bitboard >> 7) & ~file_a) attacks |= (bitboard >> 7);
  // 1 left
  if ((bitboard >> 1) & ~file_h) attacks |= (bitboard >> 1);
  
  return attacks;
}

// pre-compute king attacks
void precompute_king_attacks(unsigned long long king_attacks[64]){
  for (int pos1D = 0; pos1D < 64; ++pos1D){
    king_attacks[pos1D] = mask_king_attacks(pos1D);
  }
}

// get bishop blockers mask
unsigned long long mask_bishop_blockers(const int pos1D){
  // in this function we use the concept of magic bitboards

  // result blockers mask
  unsigned long long blockers = 0ULL;

  // bishop position
  int rank, file;
  // blockers position
  int blockers_rank, blockers_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // diagonals
  // up + right
  for (blockers_rank = rank +  1, blockers_file = file + 1; blockers_rank < 7 && blockers_file < 7; ++blockers_rank, ++blockers_file){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }
  // up + left
  for (blockers_rank = rank +  1, blockers_file = file - 1; blockers_rank < 7 && blockers_file > 0; ++blockers_rank, --blockers_file){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }
  // down + right
  for (blockers_rank = rank -  1, blockers_file = file + 1; blockers_rank > 0 && blockers_file < 7; --blockers_rank, ++blockers_file){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }
  // down + left
  for (blockers_rank = rank -  1, blockers_file = file - 1; blockers_rank > 0 && blockers_file > 0; --blockers_rank, --blockers_file){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }

  return blockers;
}

// get bishop attacks mask given blockers mask
unsigned long long mask_bishop_attacks_given_blockers(const int pos1D, const unsigned long long blockers){
  // in this function we use the concept of magic bitboards

  // result attacks mask
  unsigned long long attacks = 0ULL;

  // bishop position
  int rank, file;
  // attacks position
  int attacks_rank, attacks_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // diagonals
  // up + right
  for (attacks_rank = rank +  1, attacks_file = file + 1; attacks_rank < 8 && attacks_file < 8; ++attacks_rank, ++attacks_file){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }
  // up + left
  for (attacks_rank = rank +  1, attacks_file = file - 1; attacks_rank < 8 && attacks_file > -1; ++attacks_rank, --attacks_file){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }
  // down + right
  for (attacks_rank = rank -  1, attacks_file = file + 1; attacks_rank > -1 && attacks_file < 8; --attacks_rank, ++attacks_file){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }
  // down + left
  for (attacks_rank = rank -  1, attacks_file = file - 1; attacks_rank > -1 && attacks_file > -1; --attacks_rank, --attacks_file){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }

  return attacks;
}

// get rook blockers mask
unsigned long long mask_rook_blockers(const int pos1D){
  // in this function we use the concept of magic bitboards

  // result blockers mask 
  unsigned long long blockers = 0ULL;

  // rook position
  int rank, file;
  // blockers position
  int blockers_rank, blockers_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // straight lines
  // up
  for (blockers_rank = rank +  1 , blockers_file = file; blockers_rank < 7; ++blockers_rank){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }
  // right
  for (blockers_rank = rank , blockers_file = file + 1;blockers_file < 7; ++blockers_file){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }
  // down
  for (blockers_rank = rank -  1 , blockers_file = file; blockers_rank > 0; --blockers_rank){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }
  // left
  for (blockers_rank = rank , blockers_file = file - 1 ; blockers_file > 0; --blockers_file){
    blockers |= (1ULL << (blockers_rank * 8 + blockers_file));
  }

  return blockers;
}

// get rook attacks mask given blockers mask
unsigned long long mask_rook_attacks_given_blockers(const int pos1D, const unsigned long long blockers){
  // in this function we use the concept of magic bitboards

  // result attacks mask
  unsigned long long attacks = 0ULL;

  // rook position
  int rank, file;
  // attacks position
  int attacks_rank, attacks_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // straight lines
  // up
  for (attacks_rank = rank +  1 , attacks_file = file; attacks_rank < 8; ++attacks_rank){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }
  // right
  for (attacks_rank = rank , attacks_file = file + 1;attacks_file < 8; ++attacks_file){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }
  // down
  for (attacks_rank = rank -  1 , attacks_file = file; attacks_rank > -1; --attacks_rank){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }
  // left
  for (attacks_rank = rank , attacks_file = file - 1 ; attacks_file > -1; --attacks_file){
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & blockers){
      break;
    }
  }

  return attacks;
}

/* end of section ~ ~ ~ ~ ~ ~ */
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */



int main(){
  unsigned long long bitboard = 0ULL;
  unsigned long long blockers = 0ULL;


  print_bitboard(mask_rook_attacks_given_blockers(e4,blockers));
  
	return 0;
}
