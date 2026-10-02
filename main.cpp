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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(h, w, "Chess");
    SetWindowMinSize(320, 320);
    RenderTexture2D boardCanvas = LoadRenderTexture(w, h);
    SetTextureFilter(boardCanvas.texture, TEXTURE_FILTER_BILINEAR);
    SetTargetFPS(30); 
    int boardTiles = sizeof(board.squares);
    int boardSides = sqrt(boardTiles);
    int tileSize = std::min(h,w) / boardSides;

    int selectedSquare = -1;
    //Initializing board

    // King = 0 , Queen = 1, Pawn = 2, Bishop = 3, Knigh = 4, Rook = 5
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
    BeginTextureMode(boardCanvas);
        for (int i = 0; i < boardTiles && !WindowShouldClose(); i++)
        {
            int col = fmod(i, boardSides);
            int row = i / boardSides;
            Color boardColor = (row + col) % 2 == 0
                ? Color{240, 217, 181, 255}
                : Color{181, 136, 99, 255};
            DrawRectangle(col * tileSize, row * tileSize, tileSize, tileSize, boardColor);
            DrawText(TextFormat("%d", i), col * tileSize + 4, row * tileSize + 4, 16, BLACK);
            if(board.squares[i] != 0)
            {
                DrawTexture(pieceTextures[board.squares[i]], col * tileSize, row * tileSize, WHITE);
            }
        }
    EndTextureMode();

    std::vector <int> colorChanged;
    int color = 0;
    int turn = 0;
    bool promotion = false;
    int promotionSquare = -1;
    std::uint64_t selectedMoves = 0;
    MoveList availableMoves = generateMoves(board, turn);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F11))
            ToggleBorderlessWindowed(); // tela cheia sem bordas; F11 novamente restaura

        const float scale = std::min(
            GetScreenWidth() / static_cast<float>(w),
            GetScreenHeight() / static_cast<float>(h)
        );

        const Rectangle boardArea = {
            (GetScreenWidth() - w * scale) / 2.0f,
            (GetScreenHeight() - h * scale) / 2.0f,
            w * scale,
            h * scale
        };

        // Converte o mouse da janela para as coordenadas originais do tabuleiro.
        const Vector2 boardMouse = {
            (GetMouseX() - boardArea.x) / scale,
            (GetMouseY() - boardArea.y) / scale
        };
        //Draw only if have changes on the board
        BeginTextureMode(boardCanvas);
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
        EndTextureMode();

        BeginDrawing();
            ClearBackground(Color{22, 25, 32, 255});

            auto drawPanel = [](Rectangle area, const char* title,
                                const char* subtitle, const char* text)
            {
                if (area.width < 120 || area.height < 100)
                    return;

                DrawRectangleRounded(area, 0.08f, 8, Color{32, 37, 47, 255});

                // Impede que o texto ultrapasse o painel em janelas menores.
                BeginScissorMode(
                    static_cast<int>(area.x),
                    static_cast<int>(area.y),
                    static_cast<int>(area.width),
                    static_cast<int>(area.height)
                );

                const int left = static_cast<int>(area.x) + 16;
                const int top = static_cast<int>(area.y) + 20;

                DrawText(title, left, top, 24, RAYWHITE);
                DrawText(subtitle, left, top + 38, 18, Color{130, 190, 160, 255});
                DrawText(text, left, top + 78, 16, Color{175, 182, 195, 255});

                EndScissorMode();
            };

            const float padding = 16.0f;

            // Janela larga: painéis nas laterais.
            if (boardArea.x >= 152)
            {
                drawPanel(
                    Rectangle{
                        padding, padding,
                        boardArea.x - 2 * padding,
                        GetScreenHeight() - 2 * padding
                    },
                    "JOGADORES",
                    turn == 0 ? "Sua vez" : "Vez das pretas",
                    "Voce - Brancas\n\nComputador - Pretas\n\n"
                    "Tempo\n--:--\n\nDificuldade\nEm breve"
                );

                drawPanel(
                    Rectangle{
                        boardArea.x + boardArea.width + padding,
                        padding,
                        boardArea.x - 2 * padding,
                        GetScreenHeight() - 2 * padding
                    },
                    "PARTIDA",
                    "Historico",
                    "Nenhum lance registrado\n\n"
                    "Pecas capturadas\n--\n\n"
                    "F11 - Tela cheia"
                );
            }
            // Janela alta: painel abaixo do tabuleiro.
            else if (boardArea.y >= 132)
            {
                drawPanel(
                    Rectangle{
                        padding,
                        boardArea.y + boardArea.height + padding,
                        GetScreenWidth() - 2 * padding,
                        boardArea.y - 2 * padding
                    },
                    "PARTIDA",
                    turn == 0 ? "Sua vez - Brancas" : "Vez das pretas",
                    "Historico e pecas capturadas em breve"
                );
            }
            DrawTexturePro(boardCanvas.texture, Rectangle{0, 0, static_cast<float>(w), -static_cast<float>(h)}, boardArea, Vector2{0, 0}, 0, WHITE);
        EndDrawing();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (promotion)
            {
                Vector2 mouse =  boardMouse;
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
                        setSquare(board, promotionSquare, options[i]);
                        colorChanged.push_back(promotionSquare);
                        // Redraw every board square covered by the menu.
                        for (int row = 3; row <= 4; ++row)
                            for (int col = 1; col <= 6; ++col)
                                colorChanged.push_back(row * boardSides + col);
                        promotion = false;
                        availableMoves = generateMoves(board, turn);
                        break;
                    }
                }
                continue;
            }
            const float x = boardMouse.x;
            const float y = boardMouse.y;
            
            if((x >= 0 && x < boardSides * tileSize) && (y >= 0 && y < boardSides * tileSize))
            {
                const int col = static_cast<int>(x) / tileSize;
                const int row = static_cast<int>(y) / tileSize;
                int square = row * boardSides + col;
                int piece = board.squares[square];
                if (selectedSquare == -1)
                {
                    color = piece > 0 ? (piece < 7 ? 0 : 1) : -1;   
                    if (turn == color)
                    {
                        selectedSquare = square;
                        
                        BeginTextureMode(boardCanvas);
                        DrawRectangle(col * tileSize, row * tileSize, tileSize, tileSize, {0, 0, 255, 100});
                        for (std::size_t i = 0; i < availableMoves.count; ++i)
                        {
                            const Move& move = availableMoves.moves[i];
                            if (move.from != selectedSquare)
                                continue;

                            const std::uint64_t destinationBit = 1ULL << move.to;

                            if (selectedMoves & destinationBit)
                                continue;
                            selectedMoves |= destinationBit;
                            const int to = move.to;
                            const int destinationColumn = to % boardSides;
                            const int destinationRow = to / boardSides;

                            const int selectedPiece = board.squares[selectedSquare];

                            const bool capture = board.squares[to] != 0 ||
                                ((selectedPiece == 3 || selectedPiece == 9) &&
                                to == board.enPassantTarget);

                            Color highlight = capture
                                ? Color{200, 0, 0, 100}
                                : Color{0, 128, 0, 100};

                            DrawRectangle(
                                destinationColumn * tileSize,
                                destinationRow * tileSize,
                                tileSize, tileSize, highlight
                            );
                            colorChanged.push_back(to);
                        }

                        EndTextureMode();
                    }
                }
                else
                {
                    int from = selectedSquare;
                    int to = square;

                    if (selectedMoves & (1ULL << to)) {
                        Move move{selectedSquare, to, 0};
                        MoveResult result = makeMove(board, move);

                        colorChanged.push_back(to);

                        promotion = result.promotion;
                        if (promotion)
                            promotionSquare = to;

                        if (result.rookFrom != -1)
                        {
                            colorChanged.push_back(result.rookFrom);
                            colorChanged.push_back(result.rookTo);
                        }

                        if (result.enPassantCapturedSquare != -1)
                            colorChanged.push_back(result.enPassantCapturedSquare);

                        colorChanged.push_back(from);
                        turn = 1 - turn;

                        if (!promotion)
                            availableMoves = generateMoves(board, turn);
                    }
                    else
                    {
                        colorChanged.push_back(from);
                    }

                    selectedSquare = -1;
                    selectedMoves = 0;
                }
            }
        }
    }
    UnloadRenderTexture(boardCanvas);
    CloseWindow();  
    return 0;
}
