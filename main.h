#pragma once

#include <cstdint>
#include <array>
#include <cstddef> 

struct Board {
    std::uint64_t pieces[2][6];
    std::uint8_t squares[64];
    bool canCastleKingside[2] = {true, true};
    bool canCastleQueenside[2] = {true, true};
    int enPassantTarget = -1;
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
MoveList generateMoves(const Board& board, int color);
MoveResult makeMove(Board& board, Move& move);
