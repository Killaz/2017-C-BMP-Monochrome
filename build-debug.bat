@echo off
cmake -G "MinGW Makefiles" -DDEBUG=1 CMakeLists.txt
cmake --build .