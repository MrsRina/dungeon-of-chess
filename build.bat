:: sadly life, and useless ways to waste the life,
:: i feel to sad, likely, totally death
:: enjoy your life, and do not do shit.

@echo off

:: -S                   = source location where CMakeLists is
:: .                    = CMakeLists folder path (this path)
:: -B                   = build location, for generate makefiles
:: ./cmake-build-debug/ = build folder path
:: -G                   = makefile generator
:: if you are getting error with 'CMake Error: Could not create named generator Ninja', install ninja :cat2:

cmake -S . -B ./cmake-build-debug/ -G "Ninja"

:: --build              = build with the makefiles generated
:: ./cmake-build-debug/ = the makefiles path previously generated

cmake --build ./cmake-build-debug/

echo "-- Check the build folder
echo "-- Enjoy the cutest part of life <3

pause