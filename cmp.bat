@echo off
set "files=src\main.c src\memory.c src\global.c src\parse\expr.c src\parse\parse.c src\rational\alnat.c src\rational\rational.c src\execute\env.c src\execute\eval.c src\execute\builtin.c src\execute\term.c src\execute\stack.c"
set "test_files=src\test.c"
@REM tcc -o bin\real.exe -Isrc %files%
gcc -o bin\real.exe -Isrc -g -Wall %files%
