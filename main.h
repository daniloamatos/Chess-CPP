#pragma once

#include <cstdint>
#include <array>
#include <cstddef> 

struct Board {
    std::uint64_t byType[7]; 
    std::uint64_t byColor[2];
    std::uint8_t squares[64];
    std::uint8_t castling; // bit0 = WK, bit1 = WQ, bit2 = BK, bit3 = BQ
    std::uint8_t enPassantTarget = -1;
    std::uint8_t halfmove;   
    std::uint8_t turn;
};

enum PieceType : std::uint8_t 
{
    OCCUPIED, KING, QUEEN, PAWN, BISHOP, KNIGHT, ROOK
};

enum PieceColor : std::uint8_t 
{
    WHITEPIECE, BLACKPIECE
};

enum Castling: std::uint8_t 
{ 
    WK = 1, WQ = 2, BK = 4, BQ = 8 
};

struct Move {
    int from;
    int to;
    std::uint8_t promotion = 0; // 0 = não é promoção
};

struct MoveResult {
    int rookFrom = -1;
    int rookTo = -1;
    int enPassantCapturedSquare = -1;
    std::uint8_t promotion = 0;
};

struct MoveList {
    static constexpr std::size_t capacity = 16 * 27;

    std::array<Move, capacity> moves;
    std::size_t count = 0;

    void add(Move move)
    {
        moves.at(count) = move;
        ++count;
    }

    void clear()
    {
        count = 0;
    }
};

void setSquare(Board& board, int square, std::uint8_t piece);
MoveList generateMoves(Board& board, std::uint8_t color);
MoveResult makeMove(Board& board, Move& move);
std::array<std::uint64_t, 64> buildBishopAttacks();
