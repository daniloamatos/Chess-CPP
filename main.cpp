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
        EndDrawing();
       
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
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
                                std::cout << to << " " << (selectedSquare / 8) - (to / 8) << " " << (selectedSquare % 8) - (to % 8) << "\n" ;
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
                        board.squares[to] = piece;
                        board.squares[from] = 0;
                        movedPiece = piece;
                        movedTo[0] = col;
                        movedTo[1] = row;
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
