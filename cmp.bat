@echo off
@REM tcc -o bin\hassle.exe -Isrc %files%
gcc -o bin\hassle.exe -Isrc -g -Wall -Werror -Wpedantic ^
 src\main.c src\memory.c src\global.c src\parse\expr.c ^
 src\parse\parse.c src\rational\alnat.c src\rational\rational.c ^
 src\vm\closure.c src\vm\frame.c src\vm\heap.c src\vm\primop.c ^
 src\vm\primval.c src\vm\stack.c src\vm\term.c src\vm\vm.c ^
 src\serialize\serialize.c src\network.c src\response.c -lWs2_32
