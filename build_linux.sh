#!/bin/bash

if [[ "$1" == "show" || "$1" == "tree" ]]; then
    pattern='^\.+ (engine|src|include)'

    g++ -H -c main.cpp engine/*.cpp engine/ECS/*.cpp engine/ECS/Colliders/*.cpp -I///include/SDL2 -I///include/asio -Iengine/networking -L//mingw64/lib  -l? -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf 2>&1 | grep -E  "$pattern"

    rm -f *.o

fi

make