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

    enum CastlingRights {
        NO_CASTLING,
        WHITE_OO,
        WHITE_OOO = WHITE_OO << 1,
        BLACK_OO = WHITE_OO << 2,
        BLACK_OOO = WHITE_OO << 3,

        WHITE_CASTLING = WHITE_OO || WHITE_OOO,
        BLACK_CASTLING = BLACK_OO || BLACK_OOO,
        CASTLING_RIGHTS_NB = 16
    };

    enum PieceType {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
        PIECE_TYPE_NB = 6
    };

    enum Piece {
        
    };

}

#endif