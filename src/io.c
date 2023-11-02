//
// Created by Khan Asfi Reza on 24/10/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void handleInputParsingMemoryError(char *input, const char *errorMsg){
    free(input);
    perror(errorMsg);
    exit(1);
}


char *handleInput(){
    char *input = NULL;
    char c;
    size_t size = 0, length = 0;
    while (1) {
        c = (char) getchar();
        if (length + 1 >= size) {
            size += 10;
            char *temp = realloc(input, size);
            if (!temp) {
                handleInputParsingMemoryError(input, "Error: Memory allocation error for input parsing");
            }
            input = temp;
        }
        input[length++] = c;
        if(c == ';'){
            break;
        }
    }

    if (length > 0) {
        if (length + 1 >= size) {
            char *temp = realloc(input, length + 1);
            if (!temp) {
                handleInputParsingMemoryError(input, "Error: Memory allocation error for input parsing");
            }
            input = temp;
        }
        input[length] = '\0';
    } else {
        input = strdup("");
    }

    return input;
}
