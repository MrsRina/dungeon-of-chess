# dungeon of sdl2

Hello this is a chess game made in SDL2 and GL3, with fews of features (cursed immediate tessellator).
The game contains a primitve workable IA to play.

The IA is primitve but is smart at some point.

# license

If you are owner who did the textures, please consider making an GitHub issue, and I will add the credits for you.  
This project is learning purpose, and not even comercial, thank.

# build

There are two options for building, directly g++ command, or using CMake, I obvious recommend you use CMake.
the project now support Unix & Linux, if you want to build under Unix & Linux, open and read the commands used in `build.bat`.
The makefile generated used here was Ninja, so, I recommed installing [Ninja](https://ninja-build.org/).

Libs necessary:
[SDL 2.28.5](https://github.com/libsdl-org/SDL/releases/tag/release-2.28.5), [SDL Image 2.6.3](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.6.3), and [GLEW](https://glew.sourceforge.net/).

Also, if you are on any Linux-based OS, install with the package-manager the devel libs necessary.
```sh
# arch
sudo pacman libsd2l-dev libsdl2-image-dev libglew-dev

# ubuntu
apt install libsdl2-dev libsdl2-image-dev libglew-dev
```

# running

Download the repository using git clone or here directly, then just open `build/` folder and launch the executable.  
I do not see any reason to compile under Linux, but if you want to, please, thank you. 

# splash

![Alt text](/splash/splash_gameplay_1.png?raw=true)
![Alt text](/splash/splash_gameplay_2.png?raw=true)
