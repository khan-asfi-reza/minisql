#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global constant array
const char *const KEYWORDS[] = {
        "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE",
        "FROM", "WHERE", "SET", "VALUES", "INTO", "TABLE",
        "AND", "OR", "LIMIT", "OFFSET"
};
const size_t LEN_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

const char *INITIAL_FILE = "init.conf";


typedef enum {
    TOKEN_KEYWORD,   // SELECT, INSERT, CREATE, etc.
    TOKEN_IDENTIFIER,// table or column name
    TOKEN_STRING,    // string literal
    TOKEN_NUMBER,    // numeric constant
    TOKEN_SYMBOL,    // e.g., '*', '(', ')', ';', etc.
    TOKEN_EOF        // end of file / end of line
} TokenType;

struct {
    TokenType type;
    char* value;
} typedef Token;


int caseInsensitiveCompare(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (toupper((unsigned char)*str1) != toupper((unsigned char)*str2)) {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}


int isKeyword(const char* str) {
    for (size_t i = 0; i < LEN_KEYWORDS; i++) {
        if (caseInsensitiveCompare(str, KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}



int isNumber(const char *str) {

    if (*str == '-' || *str == '+') str++;
    int decimalFound = 0;
    while (*str) {
        if (*str == '.') {
            if (decimalFound) return 0;
            decimalFound = 1;
            str++;
            continue;
        }
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

TokenType getTokenType(const char *token){
    if (isKeyword(token)){
        return TOKEN_KEYWORD;
    } else if (token[0] == '\'' && token[strlen(token) - 1] == '\''){
        return TOKEN_STRING;
    } else if (isNumber(token)){
        return TOKEN_NUMBER;
    } else if (ispunct(token[0])){
        return TOKEN_SYMBOL;
    }
    else {
        return TOKEN_IDENTIFIER;
    }
}

void freeTokenParseMemory(char* input, char* inpArray, Token* tokens, size_t numTokens){
    free(input);
    free(inpArray);
    for (size_t i = 0; i < numTokens; i++) {
        free(tokens[i].value);
    }
    free(tokens);
}

void handleTokenParseMemError(char* input, char* inpArray, Token* tokens, size_t numTokens, const char* errorMessage) {
    perror(errorMessage);
    freeTokenParseMemory(input, inpArray, tokens, numTokens);
    exit(1);  
}

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

Token *parseInput(char *input) {
    char *inp = malloc(sizeof(char) * (strlen(input) + 1));
    strcpy(inp, input);
    size_t length = 0, prev = 0, tok_size = sizeof(Token), tok_idx=0;
    int isInStr = 0;
    char c;
    Token *tokens = malloc(tok_size);
    while (*inp) {
        c = *inp;
        if (isInStr == 0 && (c == '\n' || c == '\t')) {
            continue;
        }
        if (c == '\'') {
            isInStr = !isInStr;
        }

        if (isInStr == 0 && (c == ' ' || c == ';')) {
            if (length == prev) {
                prev++;
                continue;
            }
            char token[length - prev + 1];
            strncpy(token, input + prev, length - prev);
            token[length - prev] = '\0';
            TokenType type = getTokenType(token);
            prev = length + 1;
            tokens[tok_idx].value = malloc(strlen(token) + 1);  // Allocate memory for the token value
            if (!tokens[tok_idx].value) {
                handleTokenParseMemError(input, inp, tokens, tok_idx, "Error: Memory allocation failed for token parsing");
            }
            strcpy(tokens[tok_idx].value, token);
            tokens[tok_idx].type = type;
            tok_idx++;
            Token *temp_tok = realloc(tokens, tok_size * (tok_idx + 1));
            if(temp_tok == NULL){
                handleTokenParseMemError(input,inp, tokens, tok_idx, "Error: Memory allocation failed for token allocation");
            }
            tokens = temp_tok;
        }
        inp++;
        length++;
    }

    for (int i = 0; i < tok_idx; ++i) {
        printf("%d - %s\n", tokens[i].type, tokens[i].value);
    }
    return tokens;
}

void init(){
    FILE *file = NULL;
    file = fopen(INITIAL_FILE, "r");
    if(file == NULL){
        file = fopen(INITIAL_FILE, "w");
        if( file == NULL ){
            perror("Unable to initialize the database");
            exit(1);
        }
        fprintf(file, "INIT=1");
    }
    else{

    }

}

int main() {
    init();
//    while (1) {
//        printf("\n$>> ");
//        // Takes input
//        char *input = handleInput();
//        if(input != NULL){
//            if(caseInsensitiveCompare(input, "quit;") == 0){
//                exit(0);
//            }
//        }
//        parseInput(input);
//        // Parse the input as list of token
//        fflush(stdin);
//    }
//    Map map = new_map(100);
//    map_set(&map, "apple", "cat");
//    map_set(&map, "ide", "fle");
//    printf("%s -> %s", map.key[0], map.val[0]);
}