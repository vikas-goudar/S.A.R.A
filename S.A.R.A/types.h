#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

namespace Sara {
    using Bitboard = uint64_t;

    enum Color {
        WHITE,
        BLACK,
        COLOR_NB = 2
    };

    enum CastlingRight {
        NO_CASTLING,
        WHITE_OO,
        WHITE_OOO = WHITE_OO << 1,
        BLACK_OO = WHITE_OO << 2,
        BLACK_OOO = WHITE_OO << 3,

        WHITE_CASTLING = WHITE_OO || WHITE_OOO,
        BLACK_CASTLING = BLACK_OO || BLACK_OOO,
        CASTLING_RIGHT_NB = 16
    };

    enum PieceType {
        NO_PIECE_TYPE, ALL_PIECE_TYPE,
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
        PIECE_TYPE_NB = 8
    };

    enum Piece {
        // enum = 7, 8 not used
        // since it preserves symmetry in bit representation of piece
        // ex: 
        // W_PAWN = 0001
        // B_PAWN = 1001
        NO_PIECE,
        // White pieces have 4th bit = 0
        W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
        // Black peices have 4th bit = 1
        B_PAWN = W_PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
        PIECE_NB = 16
    };

    enum Square {
        SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
        SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
        SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
        SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
        SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
        SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
        SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
        SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8, SQ_NONE,
        
        SQ_ZERO = 0,
        SQUARE_NB = 64
    };

    enum Direction {
        NORTH = 8,
        EAST = 1,
        SOUTH = -8,
        WEST = -1,

        NORTH_EAST = NORTH + EAST,
        SOUTH_EAST = SOUTH + EAST,
        NORTH_WEST = NORTH + WEST,
        SOUTH_WEST = SOUTH + WEST
    };
    
    enum File {
        FILE_A,
        FILE_B,
        FILE_C,
        FILE_D,
        FILE_E,
        FILE_F,
        FILE_G,
        FILE_H,
        FILE_NB
    };

    enum Rank {
        RANK_1,
        RANK_2,
        RANK_3,
        RANK_4,
        RANK_5,
        RANK_6,
        RANK_7,
        RANK_8,
        RANK_NB
    };

    constexpr Direction operator+(Direction d1, Direction d2) { return Direction(int(d1) + int(d2)); }
    constexpr Direction operator*(int scale_factor, Direction d) { return Direction(int(scale_factor) * int(d)); }

    constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
    constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
    inline Square& operator+=(Square& s, Direction d) { return s = s+d; }
    inline Square& operator-=(Square& s, Direction d) { return s = s-d; };

}

#endif