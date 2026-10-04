#include "main.h"
#include <iostream>
#include <raylib.h>
#include <immintrin.h>

void setSquare(Board& board, int square, std::uint8_t piece)
{
    const std::uint64_t bit = 1ULL << square;
    const int old = board.squares[square];
    
    if (old != 0)
    {
        const int oldColor = old >> 3;
        const int oldType =  old & 7;
        board.byColor[oldColor] &= ~bit;
        board.byType[oldType] &= ~bit;
        board.byType[OCCUPIED] &= ~bit; 
    }

    board.squares[square] = piece;
    
    if (piece != 0)
    {
        const int newColor = piece >> 3; 
        const int newType = piece & 7;
        board.byColor[newColor] |= bit;
        board.byType[newType]  |= bit;
        board.byType[OCCUPIED]  |= bit;
    }
};

void addPawnMove(MoveList& moves, int color, int from, int to)
{
    const bool promotion = color == 0 ? to < 8 : to >= 56;

    if (!promotion) {
        moves.add(Move{from, to, 0});
        return;
    }

    const int options[4] = {
        color == 0 ? 2 : 8,
        color == 0 ? 6 : 12,
        color == 0 ? 4 : 10,
        color == 0 ? 5 : 11
    };

    for (int i = 0; i < 4; ++i)
        moves.add(Move{
            from, to, static_cast<std::uint8_t>(options[i])
        });
};
//2 = pawns
void generatePawnMoves(const Board& board, int color, std::uint64_t occupied, std::uint64_t enemyPieces, MoveList& moves)
{
    const int enPassant = board.enPassantTarget;

    const std::uint64_t ownPawns = (board.byColor[color] & board.byType[PAWN]);
    const std::uint64_t emptySquares = ~occupied;
    const std::uint64_t initialRank = color == 0 
        ? (0xFFULL << 48)
        : (0xFFULL << 8);

    const std::uint64_t initialPawns = ownPawns & initialRank;
    const std::uint64_t initialSinglePushes = color == 0
        ? (initialPawns >> 8) & emptySquares
        : (initialPawns << 8) & emptySquares;

    std::uint64_t singlePushes = color == 0
        ? (ownPawns >> 8) & emptySquares
        : (ownPawns << 8) & emptySquares;
    std::uint64_t doublePushes = color == 0
        ? (initialSinglePushes >> 8) & emptySquares
        : (initialSinglePushes << 8) & emptySquares;

    const int direction = color == 0 ? -8 : 8;
    
    const std::uint64_t fileA = 0x0101010101010101ULL;
    const std::uint64_t fileH = 0x8080808080808080ULL;

    std::uint64_t captureTargets = enemyPieces;

    if (enPassant >= 0 && enPassant < 64)
        captureTargets |= (1ULL << enPassant) & emptySquares;

    std::uint64_t capturesLeft = color == 0
        ? ((ownPawns & ~fileA) >> 9) & captureTargets
        : ((ownPawns & ~fileA) << 7) & captureTargets;

    std::uint64_t capturesRight = color == 0
        ? ((ownPawns & ~fileH) >> 7) & captureTargets
        : ((ownPawns & ~fileH) << 9) & captureTargets;


    while (singlePushes != 0)
    {
        const int to = __builtin_ctzll(singlePushes);
        const int from = to - direction;
        addPawnMove(moves, color, from, to);
        singlePushes &= singlePushes - 1;
    }

    while (doublePushes != 0)
    {
        const int to = __builtin_ctzll(doublePushes);
        const int from = to - 2 * direction;
        moves.add(Move{from, to, 0});
        doublePushes &= doublePushes - 1;
    }
    while (capturesLeft != 0)
    {
        const int leftDirection  = color == 0 ? -9 : 7;
        const int to = __builtin_ctzll(capturesLeft);
        const int from = to - leftDirection;
        addPawnMove(moves, color, from, to);
        capturesLeft &= capturesLeft - 1;
    }
    while (capturesRight != 0)
    {
        const int rightDirection = color == 0 ? -7 : 9;
        const int to = __builtin_ctzll(capturesRight);
        const int from = to - rightDirection;
        addPawnMove(moves, color, from, to);
        capturesRight &= capturesRight - 1;
    }
    return;
};

// //3 = bishops
// std::array<std::uint64_t, 64> buildBishopAttacks()
// {
//     std::array<std::uint64_t, 64> bishopMasks{};

//     const int rowOffsets[4]    = {-1, -1, 1, 1};
//     const int colOffsets[4] = {-1,  1,-1, 1};

//     for (int from = 0; from < 64; from++)
//     {
//         std::uint64_t mask = 0;

//         const int fromRow = from / 8;
//         const int fromColumn = from % 8;

//         for (int direction = 0; direction < 4; ++direction)
//         {
//             int row = fromRow + rowOffsets[direction];
//             int column = fromColumn + colOffsets[direction];

//             // Só casas internas: as casas da borda não precisam
//             // entrar na máscara de bloqueios.
//             while (row > 0 && row < 7 &&
//                    column > 0 && column < 7)
//             {
//                 mask |= 1ULL << (row * 8 + column);
//                 row += rowOffsets[direction];
//                 column += colOffsets[direction];
//             }
//         }

//         bishopMasks[from] = mask;
//         //const auto index = _pext_u64(board.byType[OCCUPIED], bishopMasks[from]);
//         //bishopAttackTable[from][index];
//         return bishopMasks;
//     }
// };

// const auto bishopMasks = buildBishopAttacks();

// std::array<std::array<std::uint64_t, 512>, 64> bishopAttackTable{};
// std::uint64_t bishopAttacks(int from, std::uint64_t occupied)
// {
//     const auto index = _pext_u64(occupied, bishopMasks[from]);
//     return bishopAttackTable[from][index];
// };

// void generateBishopMoves(const Board& board, int color, std::uint64_t occupied, std::uint64_t ownPieces, MoveList& moves);

//4 = knights
std::array<std::uint64_t, 64> buildKnightAttacks()
{
    std::array<std::uint64_t, 64> attacks{}; 

    const int rowOffsets[8]    = {-2, -2, -1, -1,  1, 1,  2, 2};
    const int columnOffsets[8] = {-1,  1, -2,  2, -2, 2, -1, 1};

    for (int from = 0; from < 64; ++from)
    {
        const int fromRow = from / 8;
        const int fromColumn = from % 8;

        for (int jump = 0; jump < 8; ++jump)
        {
            const int toRow = fromRow + rowOffsets[jump];
            const int toColumn = fromColumn + columnOffsets[jump];

            if (toRow >= 0 && toRow < 8 &&
                toColumn >= 0 && toColumn < 8)
            {
                const int to = toRow * 8 + toColumn;
                attacks[from] |= 1ULL << to;
            }
        }
    }

    return attacks;
};

const auto knightAttacks = buildKnightAttacks();

void generateKnightMoves(const Board& board, int color, std::uint64_t ownPieces, MoveList& moves)
{
    std::uint64_t ownKnights = board.byColor[color] & board.byType[KNIGHT];
    while (ownKnights != 0)
    {
        const int from = __builtin_ctzll(ownKnights);
        std::uint64_t destinations = knightAttacks[from] & ~ownPieces;
        while (destinations != 0)
        {
            const int to = __builtin_ctzll(destinations);
            moves.add(Move{from, to, 0});
            destinations &= destinations - 1;
        }

        ownKnights &= ownKnights - 1;
    }
    return;
};

MoveList generateMoves(Board& board, std::uint8_t color)
{
    std::uint64_t ownPieces =  board.byColor[color] ;
    std::uint64_t enemyPieces =  board.byColor[color ^ 1]; 
    std::uint64_t occupied = board.byType[OCCUPIED];

    MoveList moves;

    generatePawnMoves(board, color, occupied, enemyPieces, moves);
    generateKnightMoves(board, color, ownPieces, moves);
    //generateBishopMoves(board, color, occupied, ownPieces, moves);
    return moves;
};

MoveResult makeMove(Board& board, Move& move)
{
    MoveResult result;
    const int from = move.from;
    const int to = move.to;
    const int piece = board.squares[from];
    const int captured = board.squares[to];
    const int color = piece < 7 ? 0 : 1;
    if(piece == 1 || (piece == 6 && from == 56))
        board.castling &= ~WQ;
    if (piece == 1 || (piece == 6 && from == 63))
        board.castling &= ~WK;
    if(piece == 7 || (piece == 12 && from == 0))
        board.castling &= ~BQ;
    if (piece == 7 || (piece == 12 && from == 7))
        board.castling &= ~BK;
    if (captured == 6 && to == 56)
        board.castling &= ~WQ;
    if (captured == 6 && to == 63)
        board.castling &= ~WK;
    if (captured == 12 && to == 0)
        board.castling &= ~BQ;
    if (captured == 12 && to == 7)
        board.castling &= ~BK;
    setSquare(board, to, move.promotion != 0 ? move.promotion : piece);
    const bool reachedPromotionRank = (piece == 3 && to < 8) || (piece == 9 && to >= 56);
    setSquare(board, from, 0);
    if (std::abs(from - to) == 2 && (piece == 1 || piece == 7))
    {
        int rookFrom = (color == 0 ? 56 : 0) + (to > from ? 7 : 0) ;
        int rookTo = (from + to) / 2;
        setSquare(board, rookTo, color == 0 ? 6 : 12);
        setSquare(board, rookFrom, 0);
        result.rookFrom = rookFrom;
        result.rookTo = rookTo;
    }
    if ((piece == 3 || piece == 9) && to == board.enPassantTarget && captured == 0)
    {
        int capturedSquare = to + (color == 0 ? 8 : -8);
        setSquare(board, capturedSquare, 0);
        result.enPassantCapturedSquare = capturedSquare;
    }
    board.enPassantTarget = ((piece == 3 || piece == 9) && std::abs(from - to) == 16)
        ? (from + to) / 2
        : -1;
    result.promotion = reachedPromotionRank && move.promotion == 0;
    return result;
};