### Developing prototype of a simple 2D RTS in C++ with SDL

## Background (skip if you don't wanna know about me)
I've always wanted to develop games. Unfortunately there was no good opportunity for me to do so during my universtity undergraduate course, nor was there much job prospects where I lived. So at first I tried to learn it following some game engine tutorials.  
I tried Unity, but always got upset with all the nuts and bolts that I "don't need to know" but that are there staring me all the time.  
I thought about Unreal, but it always seemed too much overkill for any small 2D project.  
One person recommended Godot to me, but then I thought "if I want to learn things from the ground up and there's always gonna be some odd stuff that are engine-specific, why don't I just do something without an engine?".  
And that's how I ended up in the process of making my own engine. "Why would anyone do that" I hear you saying with your hands to your face. The answer is very simple: because I want to.

____

## Credit where its due
Initially based on the Birch Engine from this [tutorial on YouTube](https://www.youtube.com/watch?v=QQzAHcojEKg&list=PLhfAbcv9cehhkG7ZQK0nfIGJC_C-wSLrx) by [Carl Birch](https://www.youtube.com/@CarlBirch). I'm calling my version the [Bétula](https://en.wiktionary.org/wiki/b%C3%A9tula) Engine.   
Networking made possible by using a slightly modified version of [javidx9](https://github.com/OneLoneCoder)'s Pixel Game Engine [Networking header](https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/BiggerProjects/Networking/Parts3%264/olcPGEX_Network.h) under the OLC-3 License.  
Text font used is [kika's modified Fixedsys Excelsior](https://github.com/kika/fixedsys)  

## Build Dependencies
### Windows:  
On Windows 10 (because I don't like Windows 11) first we have to install [MSYS2](https://www.msys2.org/) (for mingw64 and g++, follow the guide on that link) because life is just easier that way.  
Using [Chocolatey](https://chocolatey.org/install), run:  
```PowerShell
choco install make
```
There is a chance that the `make` command may be linking to the wrong version after installation (I don't remember how I fixed that).  
If everything has been installed correctly, add MinGW64's lib, include, and bin paths to the environment variables in order to link them when building. They must be added to the "Path" *System* Variable.  
In my installation the paths added are these ones:  
* `C:\msys64\ming64\lib`
* `C:\msys64\ming64\include`
* `C:\msys64\ming64\bin`

Then manually download the SDL2 related MinGW ZIP files from these links:  
* [SDL2-devel-2.26.4-mingw](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.4)
* [SDL2_image-devel-2.6.3-mingw](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.6.3)
* [SDL2_mixer-devel-2.6.3-mingw](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.6.3)
* [SDL2_ttf-devel-2.20.2-mingw](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.20.2)
* [asio](https://sourceforge.net/projects/asio/)

Yes there are newer versions, but those are the ones I'm using right now.  
For the SDL libraries, grab the folders under `x86_64-w64-mingw32`, and place them under your MinGW64 directory. Mine is `C:\msys64\ming64\`. It should have the same directory tree of `share`, `lib`, `include` and `bin`, so we can just paste it there.  
For asio, copy the folder `asio-1.36.0/include` and paste it in the same include you pasted the SDL files (`C:\msys64\ming64\include` in my case), we should have an `asio` folder under include when we're done.  

The project can be built with the `make` command, but I find it easier to use my `build_windows.ps1` script since I'm mainly developing using VS Code with PowerShell.

### Linux Ubuntu and Linux Mint:
Thankfully we can just use a package manager to download the dependencies, so no need to worry about all that nonsense in the section above.  
At least on Ubuntu and Mint we can just run `sudo apt install lib<res_of_name_here>` on all of these:
* libsdl2-dev
* libsdl2-image-dev
* libsdl2-mixer-dev
* libsdl2-ttf-dev
* libasio-dev

For SDL2:
```Shell
sudo apt install libsdl2-dev
```
For SDL_image:
```Shell
sudo apt install libsdl2-image-dev
```
For SDL_mixer:
```Shell
sudo apt install libsdl2-mixer-dev
```
For SDL_ttf:
```Shell
sudo apt install libsdl2-ttf-dev
```
For asio:
```Shell
sudo apt install libasio-dev
```  

Although not as beneficial as the Windows alternative, we can also use the `build_linux.sh` to build the project (`make` works just as well).
