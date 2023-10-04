@echo off
@REM tcc -o bin\hassle.exe -Isrc %files%
gcc -o bin\hassle.exe -Isrc -g -Wall -Werror -Wpedantic ^
 src\main.c src\memory.c src\global.c src\parse\expr.c ^
 src\parse\parse.c src\rational\alnat.c src\rational\rational.c ^
 src\eval\eval.c src\eval\term.c  src\serialize\serialize.c src\network.c ^
 -lWs2_32
