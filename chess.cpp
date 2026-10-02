#include "main.h"
#include <iostream>
#include <raylib.h>

void setSquare(Board& board, int square, std::uint8_t piece)
{
    const std::uint64_t bit = 1ULL << square;
    const int old = board.squares[square];
    
    if (old != 0)
    {
        const int oldColor = (old - 1) / 6;
        const int oldPiece = (old - 1) % 6;
        board.pieces[oldColor][oldPiece] &= ~bit;
    }

    board.squares[square] = piece;
    
    if (piece != 0)
    {
        const int newColor = (piece - 1) / 6;
        const int newPiece = (piece - 1) % 6;
        board.pieces[newColor][newPiece] |= bit;
    }
};

std::uint64_t occupiedBy(const Board& board, int color)
{
    std::uint64_t occupied = 0;
    for (int type = 0; type < 6; ++type)
        occupied |= board.pieces[color][type];

    return occupied;
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
void generatePawnMoves(const Board& board, int color, std::uint64_t occupied, std::uint64_t enemyPieces, MoveList& moves)
{
    const int enPassant = board.enPassantTarget;

    const std::uint64_t ownPawns = board.pieces[color][2];
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

MoveList generateMoves(const Board& board, int color)
{
    std::uint64_t ownPieces = occupiedBy(board, color);
    std::uint64_t enemyPieces = occupiedBy(board, 1 - color);
    std::uint64_t occupied = ownPieces | enemyPieces;

    MoveList moves;

    generatePawnMoves(board, color, occupied, enemyPieces, moves);
    
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
        board.canCastleQueenside[0] = false;
    if (piece == 1 || (piece == 6 && from == 63))
        board.canCastleKingside[0] = false;
    else if(piece == 7 || (piece == 12 && from == 0))
        board.canCastleQueenside[1] = false;
    if (piece == 7 || (piece == 12 && from == 7))
        board.canCastleKingside[1] = false;
    if (captured == 6 && to == 56)
        board.canCastleQueenside[0] = false;
    if (captured == 6 && to == 63)
        board.canCastleKingside[0] = false;
    if (captured == 12 && to == 0)
        board.canCastleQueenside[1] = false;
    if (captured == 12 && to == 7)
        board.canCastleKingside[1] = false;
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