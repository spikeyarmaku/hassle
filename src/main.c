#include <stdio.h>
#include <stdlib.h>

#include "parse.h"

#define BUFFER_SIZE 1024

struct Parser create_parser(char* filename);

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("----------\nRealScript\n----------\n\n");
    if (argc > 1) {
        // There is at least one parameter
        char* filename = argv[1];
        printf("%s\n", filename);
        struct Parser parser = create_parser(filename);
        
        int go_on = 1;
        int error = 0;
        while (go_on) {
            struct Token t;
            if (get_next_token(&parser, &t) > 0) {
                printf("Error\n");
            }
            switch (t.type) {
                case OpenParen:  printf("OPEN_PAREN\n");  break;
                case CloseParen: printf("CLOSE_PAREN\n"); break;
                case Symbol:
                    printf("SYMBOL (%d): <", t.size, t.str);
                    for (int i = 0; i < t.size; i++) {
                        putchar(t.str[i]);
                    }
                    printf(">\n");
                    break;
                case Eos:        printf("EOF\n"); go_on = 0; break;
                default: printf("PANIC! Unknown token type."); break;
            }
        }
    } else {
        // There are no arguments, start a REPL
        printf("REPL TIME");
    }
    return 0;
}

struct Parser create_parser(char* file_name) {
    struct Parser p;
    FILE* fp = fopen(file_name, "r");

    // Get file size
    rewind(fp);
    long int start = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp) - start;
    printf("FILE SIZE: %d\n", size);
    rewind(fp); // fseek(fp, 0L, SEEK_SET);

    char* file_content = (char*)malloc(sizeof(char) * size);
    
    size_t chars_read = fread(file_content, sizeof(char), size, fp);
    // file_content[chars_read] = 0;

    fclose(fp);

    p.stream = file_content;
    p.size = chars_read;
    p.counter = 0;
    return p;
}

// int main()
// {
// 	char *next;
// 	const char *in = "((data da\\(\\)ta \"quot\\\\ed data\" 123 4.5)\n"
// 			" (\"data\" (!@# (4.5) \"(mo\\\"re\" \"data)\")))";

// 	expr x = parse_term(in, &next);

// 	printf("input is:\n%s\n", in);
// 	printf("parsed as:\n");
// 	print_expr(x, 0);
// 	return 0;
// }