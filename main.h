#pragma once

#include <cstdint>

struct Board {
    std::uint64_t pieces[2][6];
    std::uint8_t squares[64];
    bool canCastleKingside[2] = {true, true};
    bool canCastleQueenside[2] = {true, true};
    int enPassantTarget = -1;
};

bool validateMovement(const Board& board, int from, int to, int color);