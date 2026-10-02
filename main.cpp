#include <raylib.h>
#include <iostream>
#include "main.h"
#include <math.h>
#include <lunasvg.h>
#include <vector>
using namespace lunasvg;


int main()
{
    int h,w;
    Board board{};
    h = 800; w = 800;
    InitWindow(h, w, "raylib [core] example - basic window");
    SetTargetFPS(30); 
    int boardTiles = sizeof(board.squares);
    int boardSides = sqrt(boardTiles);
    int tileSize = std::min(h,w) / boardSides;

    bool whiteTurn = true;
    int selectedSquare = -1;
    //Initializing board

    // King = 0 , Queen = 1, Pawn = 2, Bishop = 3, Knigh = 4, Tower = 5
    auto put = [&](int color, int type, int square)
    {
        board.squares[square] = 1 + type + 6 * color;
        board.pieces[color][type] |= 1ULL << square;
    };


    int backRank[8] = {5, 4, 3, 1, 0, 3, 4, 5};

    for (int col = 0; col < 8; ++col)
    {
        put(1, backRank[col], col);      // black: a1–h1
        put(1, 2, 8 + col);              // black: a2–h2
        put(0, 2, 48 + col);             // white: a7–h7
        put(0, backRank[col], 56 + col); // white: a8–h8
    }
    const char* files[13] =
    {
        "",
        "white_king.svg", "white_queen.svg", "white_pawn.svg",
        "white_bishop.svg", "white_knight.svg", "white_rook.svg",
        "black_king.svg", "black_queen.svg", "black_pawn.svg",
        "black_bishop.svg", "black_knight.svg", "black_rook.svg"
    };

    Texture2D pieceTextures[13]{};

    for (int i = 1; i <= 12; ++i) {
        const char* path = TextFormat("vectors/chess_pieces_svg/%s", files[i]);
        auto svg = lunasvg::Document::loadFromFile(path);
        if (!svg) continue;

        auto bitmap = svg->renderToBitmap(tileSize, tileSize);
        if (bitmap.isNull()) continue;

        bitmap.convertToRGBA();
        Image image{bitmap.data(), bitmap.width(), bitmap.height(), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
        pieceTextures[i] = LoadTextureFromImage(image);
    }

    //Draw the board once, instead of drawing everyframe
    BeginDrawing();
        for (int i = 0; i < boardTiles && !WindowShouldClose(); i++)
        {
            int col = fmod(i, boardSides);
            int row = i / boardSides;
            Color boardColor = (row + col) % 2 == 0
                ? Color{240, 217, 181, 255}
                : Color{181, 136, 99, 255};
            DrawRectangle(col * tileSize, row * tileSize, tileSize, tileSize, boardColor);
            DrawText(TextFormat("%d", i), col * tileSize + 4, row * tileSize + 4, 16, BLACK);
            if(board.squares[i] >= 1 && board.squares[i] <= 6 || board.squares[i] >= 7 && board.squares[i] <= 12)
            {
                DrawTexture(pieceTextures[board.squares[i]], col * tileSize, row * tileSize, WHITE);
            }
        }
    EndDrawing();
    int movedPiece = -1;
    int movedFrom[2] = {-1, -1};
    int movedTo[2] = {-1, -1};
    std::vector <int> colorChanged;
    int color = 0;
    int turn = 0;
    bool promotion = false;
    int promotionSquare = -1;
    while (!WindowShouldClose())
    {
        //Draw only if have changes on the board
        BeginDrawing();
        while(!colorChanged.empty() && selectedSquare == -1)
        {
            int squareToChange = colorChanged.back();
            int colToChange = squareToChange % boardSides;
            int rowToChange = squareToChange / boardSides;
            Color boardColor = (colToChange + rowToChange) % 2 == 0
                    ? Color{240, 217, 181, 255}
                    : Color{181, 136, 99, 255};
            DrawRectangle(colToChange * tileSize, rowToChange * tileSize, tileSize, tileSize, boardColor);
            DrawText(TextFormat("%d", squareToChange), colToChange * tileSize + 4, rowToChange * tileSize + 4, 16, BLACK);
            int piece = board.squares[squareToChange];
            if (piece != 0)
            {
                DrawTexture(pieceTextures[piece], colToChange * tileSize, rowToChange * tileSize, WHITE);
            }
            colorChanged.pop_back();
        }
        if(movedPiece >= 1 && movedPiece <= 12)
        {
                Color boardColor = (movedFrom[0] + movedFrom[1]) % 2 == 0
                    ? Color{240, 217, 181, 255}
                    : Color{181, 136, 99, 255};
                DrawRectangle(movedFrom[0] * tileSize, movedFrom[1] * tileSize, tileSize, tileSize, boardColor);
                //DrawText(TextFormat("%d", i), movedFrom[0] * tileSize + 4, movedFrom[1] * tileSize + 4, 16, BLACK);
                DrawTexture(pieceTextures[movedPiece],movedTo[0] * tileSize, movedTo[1] * tileSize, WHITE);
                //DrawText(TextFormat("%d", i), movedTo[0] * tileSize + 4, movedTo[1] * tileSize + 4, 16, BLACK);
                movedPiece = -1;
                movedFrom[0] = -1;
                movedFrom[1] = -1;
                movedTo[0] = -1;
                movedTo[1] = -1;
        }
        if (promotion)
        {
            int options[4] = {
                color == 0 ? 2 : 8,   // queen
                color == 0 ? 6 : 12,  // rook
                color == 0 ? 4 : 10,  // bishop
                color == 0 ? 5 : 11   // knight
            };

            DrawRectangle(190, 340, 420, 120, DARKGRAY);
            for (int i = 0; i < 4; ++i)
                DrawTexture(pieceTextures[options[i]], 200 + i * 100, 350, WHITE);
        }
        EndDrawing();
       
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (promotion)
            {
                Vector2 mouse = GetMousePosition();
                for (int i = 0; i < 4; ++i)
                {
                    Rectangle area{200.0f + i * 100, 350.0f, 100.0f, 100.0f};
                    if (CheckCollisionPointRec(mouse, area))
                    {
                        int options[4] = {
                            color == 0 ? 2 : 8,
                            color == 0 ? 6 : 12,
                            color == 0 ? 4 : 10,
                            color == 0 ? 5 : 11
                        };
                        board.squares[promotionSquare] = options[i];
                        colorChanged.push_back(promotionSquare);
                        // Redraw every board square covered by the menu.
                        for (int row = 3; row <= 4; ++row)
                            for (int col = 1; col <= 6; ++col)
                                colorChanged.push_back(row * boardSides + col);
                        promotion = false;
                        break;
                    }
                }
                continue;
            }
            int x = GetMouseX();
            int y = GetMouseY();
            
            if((x >= 0 && x < boardSides * tileSize) && (y >= 0 && y < boardSides * tileSize))
            {
                int col = x / tileSize;
                int row = y / tileSize;
                int square = row * boardSides + col;
                int piece = board.squares[square];
                if (selectedSquare == -1)
                {
                    color = piece > 0 ? (piece < 7 ? 0 : 1) : -1;   
                    if (turn == color)
                    {
                        movedFrom[0] = col;
                        movedFrom[1] = row;
                        selectedSquare = square;
                        
                        BeginDrawing();
                        for (int to = 0; to < boardTiles; to++)
                        {
                            if (validateMovement(board, selectedSquare, to, color))
                            {
                                int moveCol = fmod(to, boardSides);
                                int moveRow = to / boardSides;
                                bool capture = board.squares[to] != 0;
                                Color highlight = capture
                                    ? Color{200, 0, 0, 100}
                                    : Color{0, 128, 0, 100};
                                DrawRectangle(moveCol * tileSize, moveRow * tileSize, tileSize, tileSize, highlight);
                                colorChanged.push_back(to);
                            }
                        }
                        EndDrawing();
                    }
                }
                else
                {
                    int from = selectedSquare;
                    int to = square;
                    piece = board.squares[from];
                    
                    if (validateMovement(board, from, to, color))
                    {
                        if(piece == 1 || (piece == 6 && from == 56))
                            board.canCastleQueenside[0] = false;
                        if (piece == 1 || (piece == 6 && from == 63))
                            board.canCastleKingside[0] = false;
                        else if(piece == 7 || (piece == 12 && from == 0))
                            board.canCastleQueenside[1] = false;
                        if (piece == 7 || (piece == 12 && from == 7))
                            board.canCastleKingside[1] = false;
                        board.squares[to] = piece;
                        board.squares[from] = 0;
                        movedPiece = piece;
                        movedTo[0] = col;
                        movedTo[1] = row;
                        if (std::abs(from - to) == 2 && (piece == 1 || piece == 7))
                        {
                            int rookFrom = (color == 0 ? 56 : 0) + (to > from ? 7 : 0) ;
                            int rookTo = (from + to) / 2;
                            board.squares[rookTo] = board.squares[rookFrom];
                            board.squares[rookFrom] = 0;
                            colorChanged.push_back(rookFrom);
                        }
                        if (std::abs(from - to) == 16 && (piece == 3 || piece == 9))
                        {
                            board.enPassantTarget = (from + to) / 2;
                        }
                        else if((piece == 3 || piece == 9) && to == board.enPassantTarget)
                        {
                            int capturedSquare = to + (color == 0 ? 8 : -8);
                            board.squares[capturedSquare] = 0;
                            colorChanged.push_back(capturedSquare);
                            board.enPassantTarget = -1;
                        }
                        promotion = (piece == 3 && to < 8) || (piece == 9 && to >= 56);
                        if (promotion)
                            promotionSquare = to;
                        turn = turn == 0 ? 1 : 0; 
                    }
                    else
                    {
                        movedFrom[0] = -1;
                        movedFrom[1] = -1;
                    }
                    selectedSquare = -1;
                }
            }
        }
    }
    CloseWindow();  
    return 0;
}
