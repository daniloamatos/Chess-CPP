#include "main.h"
#include <iostream>
#include <raylib.h>

bool validateMovement(const Board& board, int from, int to, int color)
{
    int piece = board.squares[from];
    int xDiff = (from / 8) - (to / 8);
    int yDiff = (from % 8) - (to % 8);
    
    if (color == 1)
        piece -= 6;
    if (piece == 1)
    {
        if (std::abs(xDiff) > 1 || std::abs(yDiff) > 1)
        {
            return false;
        }
        return color == 0 
            ? !(board.squares[to] >= 1 && board.squares[to] <= 6)
            : !(board.squares[to] >= 7 && board.squares[to] <= 12);
    }
    if (piece == 2)
    {
        //Blocks movement if is isn't on the same line or diagonal
        if (xDiff != 0 && yDiff != 0 && std::abs(xDiff) != std::abs(yDiff))
            return false;
        //Defines the direction the movement will occour. 
        //Eg: xDiff = -2, from row 4 to row 6, rowStep = +1, so it will walk DOWN; xDiff = 3, from row 5 to row 2, rowStep = -1 it will walk up 
        //(because the board matrix starts on the top left-most corner with 0 and ends on the bottom right-most corner with 63)
        int rowStep = (xDiff < 0) - (xDiff > 0);
        int colStep = (yDiff < 0) - (yDiff > 0);
        //One row up square -= 8, one row down square += 8, start at square 27 -> square 36, step = 9 (rowStep = 1 * 8) + colStep = 1
        int step = rowStep * 8 + colStep;
        //Blocks movement if enemy pieces are in the way
        for (int square = from + step; square != to; square += step)
        {
            if (board.squares[square] != 0)
                return false;
        }
        //Blocks movement if friendly pieces are in the way
        return color == 0 
            ? !(board.squares[to] >= 1 && board.squares[to] <= 6)
            : !(board.squares[to] >= 7 && board.squares[to] <= 12);
    }
    if (piece == 3){
        xDiff *=  color == 1 ? -1 : 1;
        bool initialRow = color == 0
            ? (from >= 48 && from < 56)
            : (from >= 8 && from < 16);
        //if only moving on the x-axis and one square foward
        if (xDiff == 1 && yDiff == 0)
            //if to is empty
            return board.squares[to] == 0;
        //check if is the inicial step and if is only moving on the x-axis
        else if (yDiff == 0 && xDiff == 2 && initialRow)
            //check if the to and the square in between from -> to are both empty
            return board.squares[(from + to) / xDiff] == 0 && board.squares[to] == 0;
        //check if is moving only forward once on the x-axis and once on the y-axis
        if (xDiff == 1 && (yDiff == -1 || yDiff == 1))
            //check if there is a black piece on to

            return color == 0 
            ? board.squares[to] >= 7 && board.squares[to] <= 12
            : board.squares[to] >= 1 && board.squares[to] <= 6;
    }
    if (piece == 4)
    {
        if(std::abs(yDiff) != std::abs(xDiff))
        {
            return false;
        }
        //Defines the direction the movement will occour. 
        //Eg: xDiff = -2, from row 4 to row 6, rowStep = +1, so it will walk DOWN; xDiff = 3, from row 5 to row 2, rowStep = -1 it will walk up 
        //(because the board matrix starts on the top left-most corner with 0 and ends on the bottom right-most corner with 63)
        int rowStep = (xDiff < 0) - (xDiff > 0);
        int colStep = (yDiff < 0) - (yDiff > 0);
        //One row up square -= 8, one row down square += 8, start at square 27 -> square 36, step = 9 (rowStep = 1 * 8) + colStep = 1
        int step = rowStep * 8 + colStep;
        //Blocks movement if enemy pieces are in the way
        for (int square = from + step; square != to; square += step)
        {
            if (board.squares[square] != 0)
                return false;
        }
        return color == 0 
            ? !(board.squares[to] >= 1 && board.squares[to] <= 6)
            : !(board.squares[to] >= 7 && board.squares[to] <= 12);
    }
    if (piece == 5)
    {
        if ((std::abs(xDiff) == 2 && std::abs(yDiff) == 1) || (std::abs(xDiff) == 1 && std::abs(yDiff) == 2))
        {
            return color == 0 
            ? !(board.squares[to] >= 1 && board.squares[to] <= 6)
            : !(board.squares[to] >= 7 && board.squares[to] <= 12);
        }
    }
    if (piece == 6)
    {
        if((std::abs(xDiff) != std::abs(yDiff)) && (std::abs(yDiff) == 0 || std::abs(xDiff) == 0))
        {
            //Defines the direction the movement will occour. 
            //Eg: xDiff = -2, from row 4 to row 6, rowStep = +1, so it will walk DOWN; xDiff = 3, from row 5 to row 2, rowStep = -1 it will walk up 
            //(because the board matrix starts on the top left-most corner with 0 and ends on the bottom right-most corner with 63)
            int rowStep = (xDiff < 0) - (xDiff > 0);
            int colStep = (yDiff < 0) - (yDiff > 0);
            //One row up square -= 8, one row down square += 8, start at square 27 -> square 36, step = 9 (rowStep = 1 * 8) + colStep = 1
            int step = rowStep * 8 + colStep;
            //Blocks movement if enemy pieces are in the way
            for (int square = from + step; square != to; square += step)
            {
                if (board.squares[square] != 0)
                    return false;
            }
            return color == 0 
                ? !(board.squares[to] >= 1 && board.squares[to] <= 6)
                : !(board.squares[to] >= 7 && board.squares[to] <= 12);
        }
    }
    return false;
}