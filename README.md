# Chess

A small two-player chess game written in C++. It uses raylib to draw the board and lunasvg to render the SVG piece images.

## Requirements

- A C++17 compiler
- CMake 3.16 or newer
- pkg-config and raylib
- lunasvg available as a CMake package

The repository also includes lunasvg as a Git submodule. If you cloned the repository without submodules, run `git submodule update --init --recursive`.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/chess
```

Run the executable from the repository root so it can find the piece images in `vectors/chess_pieces_svg/`.

## Controls

Click one of your pieces to see its available moves, then click a destination square. When a pawn reaches the last rank, click the piece you want to promote it to.

## Status

This is a work in progress. Check and checkmate are not implemented yet.
