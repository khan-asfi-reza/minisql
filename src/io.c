#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

/**
 * Gets input from shell and puts in a scalable size string
 * @param str String pointer
 * @param errorMsg Error Message to show
 */
void handleInputParsingMemoryError(char *str, const char *errorMsg){
    free(str);
    printError("%s", errorMsg);
    exit(1);
}

/**
 * Gets input from shell and puts in a scalable size string.
 * gets input from shell, read until ; is hit
 * @return String Input
 */
char *handleInput(){
    char *input = NULL;
    char c;
    size_t size = 0, length = 0;
    while (1) {
        c = (char) getchar();
        if (length + 1 >= size) {
            size += 10;
            // Reallocate memory to store new character in the string
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
        // Adjusting size with length and reallocation of memory
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
