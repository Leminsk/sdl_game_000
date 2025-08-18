### Developing prototype of a simple 2D RTS in C++ with SDL

Initially based on the Birch Engine from this [tutorial on YouTube](https://www.youtube.com/watch?v=QQzAHcojEKg&list=PLhfAbcv9cehhkG7ZQK0nfIGJC_C-wSLrx) by [Carl Birch](https://www.youtube.com/@CarlBirch). I'm probably gonna call my version the [Bétula](https://en.wiktionary.org/wiki/b%C3%A9tula) Engine.   
Networking made possible by using a slightly modified version of [javidx9](https://github.com/OneLoneCoder)'s Pixel Game Engine [Networking header](https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/BiggerProjects/Networking/Parts3%264/olcPGEX_Network.h) under the OLC-3 License.

## Build Dependencies
### Windows
* MSYS2 (for mingw64 and g++)
* SDL2-devel-2.26.4-mingw
* SDL2_image-devel-2.6.3-mingw
* SDL2_mixer-devel-2.6.3-mingw
* SDL2_ttf-devel-2.20.2-mingw
### Linux Ubuntu/Mint
* libsdl2-2.0-0
* libsdl2-image-2.0-0
* libsdl2-mixer-2.0-0
* libsdl2-ttf-2.0-0
* libasio-dev

Most SDL functions used here should be compatible with SDL3, but I haven't tested it yet.
