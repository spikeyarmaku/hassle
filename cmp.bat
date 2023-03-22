@echo off
@rem set "files=src\main.c src\memory.c src\global.c src\parse\expr.c src\parse\parse.c src\rational\alnat.c src\rational\rational.c src\execute\env.c src\execute\eval.c src\execute\builtin.c src\execute\term.c src\execute\term_dict.c"
set "files=src\main.c src\memory.c src\global.c src\parse\expr.c src\parse\parse.c src\rational\alnat.c src\rational\rational.c"
set "test_files=src\test.c"
@REM tcc -o bin\rsc.exe -Isrc %files%
gcc -o bin\rsc.exe -Isrc -g -Wall %files% %test_files%
