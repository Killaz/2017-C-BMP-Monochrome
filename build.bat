@echo off
cmake -G "MinGW Makefiles" -DDEBUG=0 CMakeLists.txt
mingw32-make