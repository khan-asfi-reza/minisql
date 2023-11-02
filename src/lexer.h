//
// Created by Khan Asfi Reza on 24/10/23.
//

#ifndef PARSER_H
#define PARSER_H

#define COL_MAX_SIZE 1000

#include <stdlib.h>


// Token types
typedef enum {
    TOKEN_EMPTY, //  0 EMPTY VALUE
    TOKEN_KEYWORD,   // 1 SELECT, INSERT, CREATE, etc.
    TOKEN_IDENTIFIER,// 2 table or column name
    TOKEN_STRING,    // 3 string literal
    TOKEN_NUMBER,    // 4 numeric constant
    TOKEN_SYMBOL,    // 5 e.g., '*', '(', ')', ';', etc.
    TOKEN_L_PAR,     // 6 Left parentheses '('
    TOKEN_R_PAR,     // 7 Right parentheses ')'
    TOKEN_DATA_TYPE,  // 8 Data Types as if INTEGER, BOOLEAN, FLOAT
    TOKEN_BUILT_IN_FUNC,  // Data Types as if INTEGER, BOOLEAN, FLOAT
} TokenType;

// Token structure
typedef struct {
    // Indicates if it is a keyword or identifier or anything else
    TokenType type;
    // Value that is stored
    char* value;
    // Start of syntax
    size_t start;
    // End of syntax
    size_t end;
} Token;


typedef struct {
    Token* tokens;
    size_t len;
    char* sql;
} TokenRet;

struct {
    char* display;
    Token columnToken;
    Token valueToken;
    Token dataTypeToken;
    Token symbol; // Only symbol Token
    Token* funcToken; // Only build in functions list;
    Token nextLogicalOp;
    Token defaultToken; // Default value function
    int isUnique;
} typedef Column; // Column operation




struct {
    int isInvalid; // Invalid node or not
    int isAllCol; // Select * ( Or selecting all columns )
    Token action; // Always will be a keyword
    Token table; // Token representing the table it will perform action
    Column columns[COL_MAX_SIZE]; // List of column operation
    Column filters[COL_MAX_SIZE];
    Token primaryKey; // Primary key column
    int colsLen;
    int filtersLen;
    char* sql;
    // List of filters

} typedef Node;


struct {
    Node ** nodes;
    char ** tables;
    size_t size;
} typedef NodeList;


NodeList emptyNodeList();
TokenType getTokenType(const char *token);
void freeTokenParseMemory(char* input, char* inpArray, Token* tokens, size_t numTokens);
void handleTokenParseMemError(char* input, char* inpArray, Token* tokens, size_t numTokens, const char* errorMessage);
TokenRet lexAnalyze(char *input);

Node createInvalidNode();

Node createASTNode(TokenRet tokenRet);
void insertInNodeList(NodeList *nodeList, Node *node);
Node *getNodeFromList(NodeList *nodeList, char* table);

#endif //PARSER_H

