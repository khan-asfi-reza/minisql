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


// Token Return type structure
typedef struct {
    Token* tokens; // List of tokens
    size_t len; // Number of tokens
    char* sql; // SQL String
} TokenRet;

struct {
    char* display; // AS Keyword display Example: Select id as number,
                   // anything after as keyword is a display string
    Token columnToken; // Actual Column representing token
    Token valueToken; // Value representing token
    Token dataTypeToken; // Data type representing token
    Token symbol; // Only symbol Token
    Token* funcToken; // Only built-in functions list;
    Token nextLogicalOp; // Next logic AND / OR
    Token defaultToken; // Default value function
    int isUnique; // If unique constraint is on
} typedef Column; // Column operation




struct {
    int isInvalid; // Invalid node or not
    int isAllCol; // Select * ( Or selecting all columns )
    Token action; // Always will be a keyword
    Token table; // Token representing the table it will perform action
    Column columns[COL_MAX_SIZE]; // List of column operation
    Column filters[COL_MAX_SIZE]; // Filters or where token columns
    Token primaryKey; // Primary key column
    int colsLen; // Number of columns
    int filtersLen; // Number of filters
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
void destroyNode(Node *node);
void destroyNodeList(NodeList *nodeList);
void destroyToken(Token *token);
Node createInvalidNode();

Node createASTNode(TokenRet tokenRet);
void insertInNodeList(NodeList *nodeList, Node *node);
Node *getNodeFromList(NodeList *nodeList, char* table);

#endif //PARSER_H

