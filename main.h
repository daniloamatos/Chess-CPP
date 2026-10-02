#pragma once

#include <cstdint>

struct Board {
    std::uint64_t pieces[2][6];
    std::uint8_t squares[64];
};

bool validateMovement(const Board& board, int from, int to, int color);