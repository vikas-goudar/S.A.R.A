#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// FEN dedug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "


// =====================
// Global Variables
// =====================

// globally white -> 0 and black -> 1
enum { white, black, white_black };

// board ranks inverted so that a1 gets 0 and b1 gets 1 and so on ..
enum{
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8, out_of_bounds_pos1D
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
const uint64_t rank_1 = 255ULL;
const uint64_t rank_8 = 18374686479671623680ULL;
const uint64_t file_a = 72340172838076673ULL;
const uint64_t file_b = 144680345676153346ULL;
const uint64_t file_g = 4629771061636907072ULL;
const uint64_t file_h = 9259542123273814144ULL;

// piece int refer value
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// ascii pieces
char ascii_pieces[12] = "PNBRQKpnbrqk";

// unicode pieces
char* unicode_pieces[12] = { "♟︎", "♞", "♝", "♜", "♛", "♚", "♙", "♘", "♗", "♖", "♕", "♔"};

// get refer value of ascii piece
int ascii_piece_refer_value[] = {
  ['P'] = P,
  ['N'] = N,
  ['B'] = B,
  ['R'] = R,
  ['Q'] = Q,
  ['K'] = K,
  ['p'] = p,
  ['n'] = n,
  ['b'] = b,
  ['r'] = r,
  ['q'] = q,
  ['k'] = k
};

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

uint64_t piece_bitboards[12];

// white black white_black
uint64_t piece_color_mask[3];

// side to move
int side;

// enpassant 
int enpassant_pos1D = out_of_bounds_pos1D;

// castling rights
enum { wck = 1, wcq = 2, bck = 4, bcq = 8 };
int castle;
/*
0001 -> white king can castle to the king side
0010 -> white king can castle to the queen side
0100 -> black king can castle to the king side
1000 -> black king can castle to the queen side
*/

const int bishop_occupancy_setbits[] = {
  6,  5,  5,  5,  5,  5,  5,  6, 
  5,  5,  5,  5,  5,  5,  5,  5, 
  5,  5,  7,  7,  7,  7,  5,  5, 
  5,  5,  7,  9,  9,  7,  5,  5, 
  5,  5,  7,  9,  9,  7,  5,  5, 
  5,  5,  7,  7,  7,  7,  5,  5, 
  5,  5,  5,  5,  5,  5,  5,  5, 
  6,  5,  5,  5,  5,  5,  5,  6 
};

const int rook_occupancy_setbits[] = {
  12,  11,  11,  11,  11,  11,  11,  12, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  11,  10,  10,  10,  10,  10,  10,  11, 
  12,  11,  11,  11,  11,  11,  11,  12
};

unsigned long long rook_magic_numbers[64] = {
  0xa8002c000108020ULL,
  0x6c00049b0002001ULL,
  0x100200010090040ULL,
  0x2480041000800801ULL,
  0x280028004000800ULL,
  0x900410008040022ULL,
  0x280020001001080ULL,
  0x2880002041000080ULL,
  0xa000800080400034ULL,
  0x4808020004000ULL,
  0x2290802004801000ULL,
  0x411000d00100020ULL,
  0x402800800040080ULL,
  0xb000401004208ULL,
  0x2409000100040200ULL,
  0x1002100004082ULL,
  0x22878001e24000ULL,
  0x1090810021004010ULL,
  0x801030040200012ULL,
  0x500808008001000ULL,
  0xa08018014000880ULL,
  0x8000808004000200ULL,
  0x201008080010200ULL,
  0x801020000441091ULL,
  0x800080204005ULL,
  0x1040200040100048ULL,
  0x120200402082ULL,
  0xd14880480100080ULL,
  0x12040280080080ULL,
  0x100040080020080ULL,
  0x9020010080800200ULL,
  0x813241200148449ULL,
  0x491604001800080ULL,
  0x100401000402001ULL,
  0x4820010021001040ULL,
  0x400402202000812ULL,
  0x209009005000802ULL,
  0x810800601800400ULL,
  0x4301083214000150ULL,
  0x204026458e001401ULL,
  0x40204000808000ULL,
  0x8001008040010020ULL,
  0x8410820820420010ULL,
  0x1003001000090020ULL,
  0x804040008008080ULL,
  0x12000810020004ULL,
  0x1000100200040208ULL,
  0x430000a044020001ULL,
  0x280009023410300ULL,
  0xe0100040002240ULL,
  0x200100401700ULL,
  0x2244100408008080ULL,
  0x8000400801980ULL,
  0x2000810040200ULL,
  0x8010100228810400ULL,
  0x2000009044210200ULL,
  0x4080008040102101ULL,
  0x40002080411d01ULL,
  0x2005524060000901ULL,
  0x502001008400422ULL,
  0x489a000810200402ULL,
  0x1004400080a13ULL,
  0x4000011008020084ULL,
  0x26002114058042ULL
};

unsigned long long bishop_magic_numbers[64] = {
  0x89a1121896040240ULL,
  0x2004844802002010ULL,
  0x2068080051921000ULL,
  0x62880a0220200808ULL,
  0x4042004000000ULL,
  0x100822020200011ULL,
  0xc00444222012000aULL,
  0x28808801216001ULL,
  0x400492088408100ULL,
  0x201c401040c0084ULL,
  0x840800910a0010ULL,
  0x82080240060ULL,
  0x2000840504006000ULL,
  0x30010c4108405004ULL,
  0x1008005410080802ULL,
  0x8144042209100900ULL,
  0x208081020014400ULL,
  0x4800201208ca00ULL,
  0xf18140408012008ULL,
  0x1004002802102001ULL,
  0x841000820080811ULL,
  0x40200200a42008ULL,
  0x800054042000ULL,
  0x88010400410c9000ULL,
  0x520040470104290ULL,
  0x1004040051500081ULL,
  0x2002081833080021ULL,
  0x400c00c010142ULL,
  0x941408200c002000ULL,
  0x658810000806011ULL,
  0x188071040440a00ULL,
  0x4800404002011c00ULL,
  0x104442040404200ULL,
  0x511080202091021ULL,
  0x4022401120400ULL,
  0x80c0040400080120ULL,
  0x8040010040820802ULL,
  0x480810700020090ULL,
  0x102008e00040242ULL,
  0x809005202050100ULL,
  0x8002024220104080ULL,
  0x431008804142000ULL,
  0x19001802081400ULL,
  0x200014208040080ULL,
  0x3308082008200100ULL,
  0x41010500040c020ULL,
  0x4012020c04210308ULL,
  0x208220a202004080ULL,
  0x111040120082000ULL,
  0x6803040141280a00ULL,
  0x2101004202410000ULL,
  0x8200000041108022ULL,
  0x21082088000ULL,
  0x2410204010040ULL,
  0x40100400809000ULL,
  0x822088220820214ULL,
  0x40808090012004ULL,
  0x910224040218c9ULL,
  0x402814422015008ULL,
  0x90014004842410ULL,
  0x1000042304105ULL,
  0x10008830412a00ULL,
  0x2520081090008908ULL,
  0x40102000a0a60140ULL
};

// pawn attacks table [color][pos1D]
uint64_t pawn_attacks[2][64];

// knight attacks table [pos1D]
uint64_t knight_attacks[64];

// king attacks table [pos1D]
uint64_t king_attacks[64];

// bishop attack masks
uint64_t bishop_masks[64];

// rook attack masks
uint64_t rook_masks[64];

// bishop attacks table [pos1D][occupancies]
uint64_t bishop_attacks[64][512];

// rook attacks table [pos1D][occupancies]
uint64_t rook_attacks[64][4096];

// =====================
// Bit Operations
// =====================

// get i'th bit of bitboard
static inline int get_bit(const uint64_t bitboard, const int pos1D) {
  return ((bitboard & (1ULL << pos1D)) ? 1 : 0);
}

// set i'th bit of bitboard
static inline void set_bit(uint64_t* bitboard, const int pos1D) {
  *bitboard |= (1ULL << pos1D);
}

// flip i'th bit of bitboard
static inline void flip_bit(uint64_t* bitboard, const int pos1D) {
  *bitboard ^= (1ULL << pos1D);
}

// reset i'th bit of bitboard to 0
static inline void reset_bit(uint64_t* bitboard, const int pos1D) {
  *bitboard &= ~(1ULL << pos1D);
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
  else{
    return -1; // error case
  }
  /*
  if compiler doesn't support given builtin function
  
  if (bitboard) {
    return popcount((bitboard & (~bitboard + 1)) - 1);
  }
  else{
    // handle edge error cases
  }
  */
}

// =====================
// Random 
// =====================

uint32_t random_U32_number() {
  return random();

  /*
  // if random() not supported (since it is only supported on POSIX systems)
  
  // use, XORSHIFT32 algorithm
  
  // initialize a global unsigned int variable(named state) to a random 32 bit number(non zero) first
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

// =====================
// Print 
// =====================

// print bitboard
void print_bitboard(const uint64_t bitboard) {
  printf("\n");
  for (int rank = 7; rank > -1; --rank) {
    printf("    %d  ", rank + 1);
    for (int file = 0; file < 8; ++file) {
      int pos1D = rank*8 + file;
      printf(" %d", get_bit(bitboard, pos1D));
    }
    printf("\n");
  }
  // printing files
  printf("\n        a b c d e f g h\n");
  printf("\n    Bitboard value: %llu", bitboard);
  printf("\n\n");
}

// print board
void print_board() {
  printf("\n");
  for (int rank = 7; rank > -1; --rank) {
    printf("    %d  ", rank + 1);
    for (int file = 0; file < 8; ++file) {
      int pos1D = rank*8 + file;

      // to check if piece is present
      int piece = -1;

      // loop over all piece bitboards
      for (int bb_piece = P; bb_piece <= k; ++bb_piece) {
        if (get_bit(piece_bitboards[bb_piece], pos1D)) {
          piece = bb_piece;
          break;
        }
      }
      
      // print piece according to OS
      #ifdef WIN64
        printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
      #else
        //printf(" %s",(piece == -1) ? "." : unicode_pieces[piece]);
          printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
      #endif
      
    }
    printf("\n");
  }
  printf("\n        a b c d e f g h\n");
  // print side to move
  printf("\n    Side      : %s", side ? "black" : "white");
  printf("\n    En passant: %s", (enpassant_pos1D == out_of_bounds_pos1D) ? "no" : pos1D_to_notation[enpassant_pos1D]);
  printf("\n    Castle    : %c%c%c%c", (castle & wck) ? 'K' : '-', (castle & wcq) ? 'Q' : '-', (castle & bck) ? 'k' : '-', (castle & bcq) ? 'q' : '-');
  printf("\n\n");
}

// warning - code does not test for wrong FEN notation
void parse_FEN(char* fen) {
 // reset piece bitboards
 memset(piece_bitboards, 0ULL, sizeof(piece_bitboards));

 // reset piece color mask
 memset(piece_color_mask, 0ULL, sizeof(piece_color_mask));

 // reset game variables
  side = 0;
  enpassant_pos1D = out_of_bounds_pos1D;
  castle = 0;

  // setting up pieces on board
  // error checking not included !! 
  for (int rank = 7; rank > -1 && *fen && *fen != ' '; --rank) {
    for (int file = 0; file < 8 && *fen && *fen != ' '; ) {
      int pos1D = rank*8 + file;
      // if it's a piece
      if ((*fen >= 'b' && *fen <= 'r') || (*fen >= 'B' && *fen <= 'R')) {
        // piece type
        int piece = ascii_piece_refer_value[*fen];

        // set piece on corresponding piece bitboard
        set_bit(&piece_bitboards[piece], pos1D);

        ++file;
        ++fen;
      }

      // unoccupied positions
      else if (*fen >= '1' && *fen <= '9') {
        int offset = *fen - '0';

        //skip offset positions
        file += offset;
        ++fen;
      }

      else if(*fen == '/') {
        ++fen;
      }

      else {
        // error case -> unkown char
        ++fen;
      }
    }
  }

  // initializing game variables
  // error checking not included !!
  // side to move
  ++fen;
  (*fen == 'w') ? (side = white) : (side = black);
  ++fen;

  // castle rights
  ++fen;
  while (*fen != ' ') {
    switch (*fen) {
      case 'K': castle |= wck; break;
      case 'Q': castle |= wcq; break;
      case 'k': castle |= bck; break;
      case 'q': castle |= bcq; break;
      case '-': break;
    }
    ++fen;
  }

  //en passant
  ++fen;
  if(*fen != '-') {
    int file = *fen - 'a';
    ++fen;
    int rank = *fen - '1';
    enpassant_pos1D = rank*8 + file;
  }
  else {
    enpassant_pos1D = out_of_bounds_pos1D;
  }

  // setting up occupancy masks
  for (int piece = P; piece <= K; ++piece) {
    piece_color_mask[white] |= piece_bitboards[piece];
  }
  for (int piece = p; piece <= k; ++piece) {
    piece_color_mask[black] |= piece_bitboards[piece];
  }
  piece_color_mask[white_black] |= (piece_color_mask[white] | piece_color_mask[black]);
}

// =====================
// Attacks
// =====================

// get pawn attacks mask
uint64_t mask_pawn_attacks(const int color, const int pos1D) {
  
  // result attacks mask
  uint64_t attacks = 0ULL;
  
  // set piece on board
  uint64_t bitboard = 0ULL;
  set_bit(&bitboard, pos1D);
  
// white pawns
  if (!color) {
    if (bitboard & ~file_h) {
      attacks |= (bitboard << 9);
    }
    if (bitboard & ~file_a) {
      attacks |= (bitboard << 7);
    }
    
  }
  // black pawns
  else{
    if (bitboard & ~file_h) {
      attacks |= (bitboard >> 7);
    }
    if (bitboard & ~file_a) {
      attacks |= (bitboard >> 9);
    }
  }

  return attacks;
}

// pre-compute pawn attacks
void precompute_pawn_attacks(uint64_t pawn_attacks[2][64]) {
  for (int pos1D = 0; pos1D < 64; ++pos1D) {
    pawn_attacks[white][pos1D] = mask_pawn_attacks(white,pos1D);
    pawn_attacks[black][pos1D] = mask_pawn_attacks(black,pos1D);
  }
}

// get knight attacks mask
uint64_t mask_knight_attacks(const int pos1D) {
  // result attacks mask
  uint64_t attacks = 0ULL;
  
  // set piece on board
  uint64_t bitboard = 0ULL;
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
void precompute_knight_attacks(uint64_t knight_attacks[64]) {
  for (int pos1D = 0; pos1D < 64; ++pos1D) {
    knight_attacks[pos1D] = mask_knight_attacks(pos1D);
  }
}

// get king attacks mask
uint64_t mask_king_attacks(const int pos1D) {
  // result attacks mask
  uint64_t attacks = 0ULL;
  
  // set piece on board
  uint64_t bitboard = 0ULL;
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
void precompute_king_attacks(uint64_t king_attacks[64]) {
  for (int pos1D = 0; pos1D < 64; ++pos1D) {
    king_attacks[pos1D] = mask_king_attacks(pos1D);
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
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }
  // up + left
  for (occupancy_rank = rank +  1, occupancy_file = file - 1; occupancy_rank < 7 && occupancy_file > 0; ++occupancy_rank, --occupancy_file) {
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }
  // down + right
  for (occupancy_rank = rank -  1, occupancy_file = file + 1; occupancy_rank > 0 && occupancy_file < 7; --occupancy_rank, ++occupancy_file) {
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }
  // down + left
  for (occupancy_rank = rank -  1, occupancy_file = file - 1; occupancy_rank > 0 && occupancy_file > 0; --occupancy_rank, --occupancy_file) {
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }

  return occupancy;
}

// get bishop attacks mask given occupancy mask
uint64_t mask_bishop_attacks_given_occupancy(const int pos1D, const uint64_t occupancy) {
  // result attacks mask
  uint64_t attacks = 0ULL;

  // bishop position
  int rank, file;
  // attacks position
  int attacks_rank, attacks_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // diagonals
  // up + right
  for (attacks_rank = rank +  1, attacks_file = file + 1; attacks_rank < 8 && attacks_file < 8; ++attacks_rank, ++attacks_file) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }
  // up + left
  for (attacks_rank = rank +  1, attacks_file = file - 1; attacks_rank < 8 && attacks_file > -1; ++attacks_rank, --attacks_file) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }
  // down + right
  for (attacks_rank = rank -  1, attacks_file = file + 1; attacks_rank > -1 && attacks_file < 8; --attacks_rank, ++attacks_file) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }
  // down + left
  for (attacks_rank = rank -  1, attacks_file = file - 1; attacks_rank > -1 && attacks_file > -1; --attacks_rank, --attacks_file) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }

  return attacks;
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
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }
  // right
  for (occupancy_rank = rank , occupancy_file = file + 1;occupancy_file < 7; ++occupancy_file) {
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }
  // down
  for (occupancy_rank = rank -  1 , occupancy_file = file; occupancy_rank > 0; --occupancy_rank) {
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }
  // left
  for (occupancy_rank = rank , occupancy_file = file - 1 ; occupancy_file > 0; --occupancy_file) {
    occupancy |= (1ULL << (occupancy_rank * 8 + occupancy_file));
  }

  return occupancy;
}

// get rook attacks mask given occupancy mask
uint64_t mask_rook_attacks_given_occupancy(const int pos1D, const uint64_t occupancy) {

  // result attacks mask
  uint64_t attacks = 0ULL;

  // rook position
  int rank, file;
  // attacks position
  int attacks_rank, attacks_file;

  rank = pos1D / 8;
  file = pos1D % 8;

  // straight lines
  // up
  for (attacks_rank = rank +  1 , attacks_file = file; attacks_rank < 8; ++attacks_rank) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }
  // right
  for (attacks_rank = rank , attacks_file = file + 1;attacks_file < 8; ++attacks_file) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }
  // down
  for (attacks_rank = rank -  1 , attacks_file = file; attacks_rank > -1; --attacks_rank) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }
  // left
  for (attacks_rank = rank , attacks_file = file - 1 ; attacks_file > -1; --attacks_file) {
    attacks |= (1ULL << (attacks_rank * 8 + attacks_file));
    if ((1ULL << (attacks_rank * 8 + attacks_file)) & occupancy) {
      break;
    }
  }

  return attacks;
}

// generates ith combination from all possible occupancy
uint64_t ith_occupancy_combination(const int ith_combination, const int setbits_in_mask, uint64_t occupancy_mask) {
  // ith combination ranges from 0 .. 2^(bits in mask) - 1

  // result ith occupancy mask
  uint64_t ith_occupancy_mask = 0ULL;

  for (int i = 0; i < setbits_in_mask; ++i) {
    // LSB index
    int pos1D = LSB_index(occupancy_mask);

    // remove LSB
    reset_bit(&occupancy_mask, pos1D);

    // add occupancy if its in ith combination
    if (ith_combination & (1 << i)) {
      ith_occupancy_mask |= (1ULL << pos1D);
    }
  }

  return ith_occupancy_mask;
}

// find a candidate for magic number that can be used for magic bitboard for bishops and rooks
uint64_t magic_number(const int pos1D, const int occupancy_mask_setbits, const int is_bishop) {
  // store all ith occupancy combinations
  uint64_t occupancy[4096];

  // store all attacks given ith occupancy combination
  uint64_t attacks[4096];

  // to check wether the current indexed as already been reffered by another attack
  uint64_t attacks_used[4096];

  uint64_t occupancy_mask = is_bishop ? mask_bishop_occupancy(pos1D): mask_rook_occupancy(pos1D);

  int num_occupancy_combination = 1 << occupancy_mask_setbits;

  // loop through all ith occupancy combination
  for (int ith = 0; ith < num_occupancy_combination; ++ith) {
    occupancy[ith] = ith_occupancy_combination(ith, occupancy_mask_setbits, occupancy_mask);
    attacks[ith] = is_bishop ? mask_bishop_attacks_given_occupancy(pos1D, occupancy[ith]) : mask_rook_attacks_given_occupancy(pos1D, occupancy[ith]);
  }

  // loop to find candidate magic
  for (int random_count = 0; random_count < 10e8; ++random_count) {
    uint64_t candidate_magic = random_U64_number_low_population();

    if (popcount((occupancy_mask * candidate_magic) & 0xFF00000000000000ULL) < 6) continue;
    memset(attacks_used, 0ULL, sizeof(attacks_used));

    int i,fail;
    for (i = 0, fail = 0; !fail && i < num_occupancy_combination; ++i) {
      int hash_index = (int)((occupancy[i] * candidate_magic) >> (64 - occupancy_mask_setbits));
      
      
      if (attacks_used[hash_index] == 0ULL) {
        attacks_used[hash_index] = attacks[i];
      }
      
      else if (attacks_used[hash_index] != attacks[i]) {
        fail = 1;
      }
    }

    if (!fail) {
      return candidate_magic;
    }
  }
  printf("Magic candidate failed");
  return 0ULL;
}

// get bishop attacks
static inline uint64_t get_bishop_attacks(int pos1D, uint64_t occupancy) {
  uint64_t rel_occupancy = occupancy & bishop_masks[pos1D];
  int hash_index = (int)((rel_occupancy * bishop_magic_numbers[pos1D]) >> (64 - bishop_occupancy_setbits[pos1D]));

  return bishop_attacks[pos1D][hash_index];
}

// get rook attacks
static inline uint64_t get_rook_attacks(int pos1D, uint64_t occupancy) {
  uint64_t rel_occupancy = occupancy & rook_masks[pos1D];
  int hash_index = (int)((rel_occupancy * rook_magic_numbers[pos1D]) >> (64 - rook_occupancy_setbits[pos1D]));

  return rook_attacks[pos1D][hash_index];
}

// get queen attacks
static inline uint64_t get_queen_attacks(int pos1D, uint64_t occupancy) {
  return get_bishop_attacks(pos1D, occupancy) | get_rook_attacks(pos1D, occupancy);
}

// is square attacked by the given side
static inline int is_square_attacked(int pos1D, int side) {
  // if square is attacked by white pawns
  if ((side == white) && (pawn_attacks[black][pos1D] & piece_bitboards[P])) return 1;

  // if square is attacked by black pawns
  if ((side == black) && (pawn_attacks[white][pos1D] & piece_bitboards[p])) return 1;

  // if square is attacked by knight
  if (knight_attacks[pos1D] & ((side == white) ? piece_bitboards[N] : piece_bitboards[n])) return 1;

  // if square is attacked by king
  if (king_attacks[pos1D] & ((side == white) ? piece_bitboards[K] : piece_bitboards[k])) return 1;

  // if square is attacked by bishop
  if (get_bishop_attacks(pos1D, piece_color_mask[white_black]) & ((side == white) ? piece_bitboards[B] : piece_bitboards[b])) return 1;

  // if square is attacked by rook
  if (get_rook_attacks(pos1D, piece_color_mask[white_black]) & ((side == white) ? piece_bitboards[R] : piece_bitboards[r])) return 1;

  // if square is attacked by queen  
  if (get_queen_attacks(pos1D, piece_color_mask[white_black]) & ((side == white) ? piece_bitboards[Q] : piece_bitboards[q])) return 1;


  return 0;
}

// prints attacked squares
void print_attacked_squares(int side) {
  for (int rank = 7; rank > -1; --rank) {
    printf("    %d  ", rank + 1);
    for (int file = 0; file < 8; ++file) {
      int pos1D = rank*8 + file;
      printf(" %d", is_square_attacked(pos1D, side) ? 1 : 0); 
    }
    printf("\n");
  }
  printf("\n        a b c d e f g h\n");
}


// =====================
// Init 
// =====================

void init_magic_numbers() {
  printf("Rook \n");
  for (int pos1D = 0; pos1D < 64; pos1D++) {
    printf("0x%llxULL,\n", magic_number(pos1D, rook_occupancy_setbits[pos1D], 0));
  }
  printf("Bishop \n");
  for (int pos1D = 0; pos1D < 64; pos1D++) {
    printf("0x%llxULL,\n", magic_number(pos1D, bishop_occupancy_setbits[pos1D], 1));
  }
}

void init_piece_occupancy_setbits() {
  printf("rook\n");
  for (int rank = 7; rank > -1; --rank) {
    for (int file = 0; file < 8; ++file) {
      int pos1D = 8*rank + file;
      printf(" %d,", popcount(mask_rook_occupancy(pos1D)));
    }
    printf("\n");
  }

  printf("bishop");
  for (int rank = 7; rank > -1; --rank) {
    for (int file = 0; file < 8; ++file) {
      int pos1D = 8*rank + file;
      printf(" %d,", popcount(mask_bishop_occupancy(pos1D)));
    }
    printf("\n");
  }

}

void init_leapers() {
  for (int square = 0; square < 64; square++) {
    // init pawn attacks
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    
    // init knight attacks
    knight_attacks[square] = mask_knight_attacks(square);
    
    // init king attacks
    king_attacks[square] = mask_king_attacks(square);
  }
}

void init_sliders() {
  for (int pos1D = 0; pos1D <64; ++pos1D) {
    int set_bits;
    int possible_combinations;
    int hash_index;
    // bishop
    // occupancy mask
    bishop_masks[pos1D] = mask_bishop_occupancy(pos1D);
    set_bits = bishop_occupancy_setbits[pos1D];
    possible_combinations = (1 << set_bits);

    // iterating through all possible occupancies 
    for (int ith = 0; ith < possible_combinations; ++ith) {
      uint64_t ith_occupancy = ith_occupancy_combination(ith, set_bits, bishop_masks[pos1D]);

      // hashed index
      hash_index = (ith_occupancy * bishop_magic_numbers[pos1D]) >> (64 - set_bits);

      // set bishop attacks
      bishop_attacks[pos1D][hash_index] = mask_bishop_attacks_given_occupancy(pos1D, ith_occupancy);
      
    }
    // rook
    // occupancy mask
    rook_masks[pos1D] = mask_rook_occupancy(pos1D);
    set_bits = rook_occupancy_setbits[pos1D];
    possible_combinations = (1 << set_bits);

    // iterating through all possible occupancies 
    for (int ith = 0; ith < possible_combinations; ++ith) {
      uint64_t ith_occupancy = ith_occupancy_combination(ith, set_bits, rook_masks[pos1D]);

      // hashed index
      hash_index = (ith_occupancy * rook_magic_numbers[pos1D]) >> (64 - set_bits);

      // set rook attacks
      rook_attacks[pos1D][hash_index] = mask_rook_attacks_given_occupancy(pos1D, ith_occupancy);
      

    }
  }
}

void init() {
  init_leapers();
  // init_piece_occupancy_setbits(); -> stored in array already
  // init_magic_numbers(); -> stored in array already
  init_sliders();
}

// =====================
// Main
// =====================

int main() {
  init();
  
  
  parse_FEN(tricky_position);
  //parse_FEN(start_position);
  print_board();
  print_bitboard(piece_color_mask[white_black]);

  print_attacked_squares(white);
  
	return 0;
}
