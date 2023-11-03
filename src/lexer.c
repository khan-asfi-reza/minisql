#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "lexer.h"
#include "const.h"


TokenType getTokenType(const char *token){
    if (isKeyword(token)){
        return TOKEN_KEYWORD;
    }
    else if (isDataType(token)){
        return TOKEN_DATA_TYPE;
    }
    else if (isBuiltInFunc(token)){
        return TOKEN_BUILT_IN_FUNC;
    }
    else if (token[0] == '('){
        return TOKEN_L_PAR;
    }
    else if(token[0] == ')'){
        return TOKEN_R_PAR;
    }
    else if (token[0] == '\'' && token[strlen(token) - 1] == '\''){
        return TOKEN_STRING;
    }
    else if (isNumber(token)){
        return TOKEN_NUMBER;
    }
    else if (isSymbol(token)){
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


TokenRet createEmptyTokenRet(){
    TokenRet t = {NULL, 0};
    return t;
}


TokenRet createEmptyTokenRetAfterFree(char *input, Token* tokens, size_t token_len){
    free(input);
    for (size_t i = 0; i < token_len; i++) {
        free(tokens[i].value);
    }
    free(tokens);
    return createEmptyTokenRet();
}

void printErrorMsg(const char *input, size_t start, const char* extra){
    printf("\033[1;31m");
    printf("%s%s` At point %ld; %s\n", SYNTAX_ERROR_START, input, start, extra);
    size_t len = strlen(SYNTAX_ERROR_START);
    for (int i = 0; i < len + start; ++i) {
        printf(" ");
    }
    printf("^");
    printf("\033[0m\n");
}

TokenRet lexAnalyze(char *input) {
    char *inp = malloc(sizeof(char) * (strlen(input) + 1));
    strcpy(inp, input);
    size_t length = 0, prev = 0, tok_size = sizeof(Token), tok_idx = 0;
    int isInStr = 0, isInPar=0;
    size_t strStart = -1, parStart = -1;
    char c;
    Token *tokens = malloc(tok_size);
    if (!tokens) {
        handleTokenParseMemError(input, inp, tokens, tok_idx, "Error: Memory allocation failed for token storage");
    }

    while (*inp) {
        c = *inp;

        if(isInStr == 0){
            if(c == '(' || c == ')'){
                isInPar = !isInPar;
                if(isInPar){
                    parStart = length;
                }
                else{
                    parStart = -1;
                }
            }
        }

        // Throw parsing errors
        if(isInStr && c==';'){
            printErrorMsg(input, strStart, "");
            return createEmptyTokenRetAfterFree(input, tokens, tok_idx);
        }

        if(isInPar && c==';'){
            printErrorMsg(input, parStart, "");
            return createEmptyTokenRetAfterFree(input, tokens, tok_idx);
        }

        // Skip new line and tabs, which will cause parsing error
        if (isInStr == 0 && (c == '\n' || c == '\t')) {
            inp++;
            prev++;
            continue;
        }

        // Toggle isInStr flag when encountering a single quote.
        // If isInStr = 1 , set to 0
        // Otherwise set to 1
        if (c == '\'') {
            isInStr = !isInStr;
            if(isInStr){
                strStart = length;
            }
            else{
                strStart = -1;
            }
        }

        if (isInStr == 0 && (c == ' ' || c == ';' || isSpecialPunct(c))) {
            // If the token that is being selected is a full string, not a punctuation then
            if (length != prev) {
                char token[length - prev + 1];
                strncpy(token, input + prev, length - prev);
                token[length - prev] = '\0';
                TokenType type = getTokenType(token);
                char* string = NULL;

                tokens[tok_idx].start = prev;
                tokens[tok_idx].value = malloc(strlen(token) + 1);
                if (!tokens[tok_idx].value) {
                    handleTokenParseMemError(input, inp, tokens, tok_idx, "Error: Memory allocation failed for token parsing");
                }
                if(type == TOKEN_KEYWORD || type == TOKEN_DATA_TYPE || type == TOKEN_BUILT_IN_FUNC){
                    stringToLower(token);
                }
                if(type == TOKEN_STRING){
                    string = escapeCommas(token);
                }
                if(string != NULL){
                    strcpy(tokens[tok_idx].value, string);
                    free(string);
                }
                else{
                    strcpy(tokens[tok_idx].value, token);
                }
                tokens[tok_idx].type = type;
                tokens[tok_idx].end = length;
                tok_idx++;
                Token *temp_tok = realloc(tokens, tok_size * (tok_idx + 1));
                if(temp_tok == NULL){
                    handleTokenParseMemError(input, inp, tokens, tok_idx, "Error: Memory allocation failed for token allocation");
                }
                tokens = temp_tok;
            }

            // Special handling for punctuations for example '(' , ')' , ',' , '.'
            if(isSpecialPunct(c) && c != ';') {
                char token[2] = {c, '\0'};
                TokenType type = getTokenType(token);
                tokens[tok_idx].start = length;
                tokens[tok_idx].value = malloc(2);
                if (!tokens[tok_idx].value) {
                    handleTokenParseMemError(input, inp, tokens, tok_idx, "Error: Memory allocation failed for token parsing");
                }
                strcpy(tokens[tok_idx].value, token);
                tokens[tok_idx].type = type;
                tokens[tok_idx].end = length + 1;
                tok_idx++;
                Token *temp_tok = realloc(tokens, tok_size * (tok_idx + 1));
                if(temp_tok == NULL){
                    handleTokenParseMemError(input, inp, tokens, tok_idx, "Error: Memory allocation failed for token allocation");
                }
                tokens = temp_tok;
            }
            prev = length + 1;
        }

        inp++;
        length++;
    }

//    // For demonstration purposes: printing the tokens.
//    for (int i = 0; i < tok_idx; ++i) {
//        printf("%d - %s - %ld - %ld\n", tokens[i].type, tokens[i].value, tokens[i].start, tokens[i].end);
//    }

    TokenRet tokenRet = {tokens, tok_idx, input};
    return tokenRet;
}

/**
 SELECT
 ---------
 General select statement examples:
 -> Select * from table;
 -> Select col, col2, col3, from table;
 -> Select col as name, col2 as name, col3 as name from table;
 -> Select col from table where col = Jjz;

 CREATE
 ---------
 Creating table examples:
 -> Create table table1(id integer, name varchar(255) not null, email text not null)

 UPDATE
 ---------
 Update table examples

 -> Update table set col1=1, col2=2, col3=3 where col1 = value;

 DELETE
 ---------
 Delete row examples:
 -> Delete from table1 where col1 = 1;


 INSERT
 ---------
 Insert row examples:
 -> Insert into table1(col1, col2, col3) values('val1', 'val2', 'val3')

**/


Node createInvalidNode(){
    Node node;
    node.isInvalid = 1;
    return node;
}


Node handleWhereClauseError(const char* sql, size_t start){
    printErrorMsg(sql, start, "invalid `where` clause");
    return createInvalidNode();
}

int isPostColumnSelector(Token action, Token table, Token *tokens, size_t i){
    if(caseInsensitiveCompare(action.value, "DELETE") == 0){
        return 0;
    }
    if(
            isSelectKeyword(action.value) && tokens[i].type == TOKEN_IDENTIFIER ||
            (
                    caseInsensitiveCompare(action.value, "UPDATE") == 0 &&
                    caseInsensitiveCompare(tokens[i].value, "SET") == 0
            ) ||
            (
                    caseInsensitiveCompare(action.value, "CREATE") == 0 && table.type != TOKEN_EMPTY &&
                    tokens[i].type == TOKEN_L_PAR
            ) ||
            (
                    caseInsensitiveCompare(action.value, "INSERT") == 0 &&
                            tokens[i].type == TOKEN_L_PAR
            )
    )
    {
        return 1;
    }
    return 0;
}

Token emptyToken(){
    Token token;
    token.type = TOKEN_EMPTY;
    return token;
}

Node createASTNode(TokenRet tokenRet){
    Node node;

    node.colsLen = 0;
    node.filtersLen = 0;
    node.isInvalid = 0;
    node.isAllCol = 0;

    size_t i = 0;
    Token action = emptyToken();
    Token table = emptyToken();
    Token primaryKey = emptyToken();
    size_t len = tokenRet.len;
    Token *tokens = tokenRet.tokens;
    node.table = table;
    node.action = action;
    node.primaryKey = primaryKey;
    int colsSet = 0;
    while(i < len){
        Token cur = tokens[i];
        if(i == 0){
            if(cur.type != TOKEN_KEYWORD){
                printErrorMsg(tokenRet.sql, cur.start, "");
                return createInvalidNode();
            }

            node.action = cur;
            action = cur;
        }
        else{

            if(cur.type == TOKEN_SYMBOL && strcmp(cur.value, "*") == 0 && isSelectKeyword(action.value) == 1){
                node.isAllCol = 1;
                colsSet = 1;
            }

            else if(i == 1 && (isUpdateKeyword(action.value) || isDeleteKeyword(action.value))){
                if(tokens[i].type != TOKEN_IDENTIFIER){
                    if(tokens[i+1].type == TOKEN_KEYWORD){
                        printErrorMsg(tokenRet.sql, tokens[i+1].start, "Invalid table name, SQL Keywords cannot be a table.");
                    }
                    else{
                        printErrorMsg(tokenRet.sql, tokens[i+1].start, "Invalid table name.");
                    }
                    return createInvalidNode();
                }
                node.table = tokens[i];
                table = tokens[i];
            }

            else if(isPreTableSelectorKeyword(cur.value)){
                // Show error
                if (tokens[i+1].type != TOKEN_IDENTIFIER){
                    if(tokens[i+1].type == TOKEN_KEYWORD){
                        printErrorMsg(tokenRet.sql, tokens[i+1].start, "Invalid table name, SQL Keywords cannot be a table.");
                    }
                    else{
                        printErrorMsg(tokenRet.sql, tokens[i+1].start, "Invalid table name.");
                    }
                    return createInvalidNode();
                }
                node.table = tokens[i+1];
                table = tokens[i+1];
                i++;
            }

            else if(colsSet == 0 && isPostColumnSelector(action, table, tokens, i)){
                int isInPar = 0;
                if(cur.type == TOKEN_L_PAR){
                    isInPar = 1;
                }
                if(tokens[i].type == TOKEN_KEYWORD || tokens[i].type == TOKEN_L_PAR){
                    i++;
                }
                if(tokens[i+1].type == TOKEN_L_PAR){
                    isInPar = 1;
                    i+=2;
                }
                int isInsert = 0;
                if(caseInsensitiveCompare(action.value, "INSERT") == 0){
                    isInsert = 1;
                }
                int start = (int)i;
                TokenType  prevType = TOKEN_EMPTY;
                int cols_index = 0;

                while (i < len){
                    if( tokens[i].type == TOKEN_R_PAR ||
                        tokens[i].type == TOKEN_KEYWORD){
                        if(caseInsensitiveCompare(tokens[i].value, "AS") == 0){
                            if(i+1 < len && tokens[i+1].type == TOKEN_IDENTIFIER){
                                node.columns[i].display = tokens[i].value;
                            }
                            else{
                                printErrorMsg(tokenRet.sql, tokens[i].start, "Invalid column as");
                                return createInvalidNode();
                            }
                        }
                        else{
                            if(tokens[i].type == TOKEN_KEYWORD){
                                i--;
                            }
                            break;
                        }
                    }
                    if(start == i && tokens[i].type != TOKEN_IDENTIFIER){
                        printErrorMsg(tokenRet.sql, tokens[i].start, "Invalid column name");
                        return createInvalidNode();
                    }

                    if(tokens[i].type == TOKEN_IDENTIFIER){
                        if(prevType != TOKEN_EMPTY){
                            printErrorMsg(tokenRet.sql, tokens[i].start, "Invalid select statement");
                            return createInvalidNode();
                        }
                        node.columns[cols_index].columnToken = tokens[i];
                        prevType = TOKEN_IDENTIFIER;
                    }

                    if(tokens[i].type == TOKEN_DATA_TYPE){
                        if(prevType != TOKEN_IDENTIFIER){
                            printErrorMsg(
                                    tokenRet.sql,
                                    tokens[i].start,
                                    "Data type order mismatch, data type must be followed by the name of the column"
                                    );
                            return createInvalidNode();
                        }
                        node.columns[cols_index].dataTypeToken = tokens[i];
                        prevType = TOKEN_DATA_TYPE;
                    }


                    if(tokens[i].type == TOKEN_BUILT_IN_FUNC){
                        if(prevType != TOKEN_BUILT_IN_FUNC && prevType != TOKEN_DATA_TYPE){
                            printErrorMsg(tokenRet.sql, tokens[i].start, "Column options must be followed by the column data type");
                            return createInvalidNode();
                        }

                        if(caseInsensitiveCompare(tokens[i].value, "UNIQUE") == 0){
                            node.columns[cols_index].isUnique = 1;
                            prevType = TOKEN_BUILT_IN_FUNC;
                        }

                        else if(caseInsensitiveCompare(tokens[i].value, "DEFAULT") == 0 && i < len - 1 && tokens[i+1].type == TOKEN_BUILT_IN_FUNC){
                            if(isValueFunc(tokens[i+1].value)){
                                node.columns[cols_index].defaultToken = tokens[i+1];
                                i++;
                            }
                            else{
                                const char* err = " :is not a valid default";
                                char *newString = (char *)malloc(strlen(tokens[i].value) + strlen(err) + 1);
                                sprintf(newString, "%s%s", tokens[i].value, err);
                                printErrorMsg(tokenRet.sql, tokens[i].start, newString);
                                free(newString);
                                return createInvalidNode();
                            }
                        }
                        else if(caseInsensitiveCompare(tokens[i].value, "PRIMARY") == 0 && i < len - 1 &&
                                caseInsensitiveCompare(tokens[i+1].value, "KEY") == 0){
                            node.primaryKey = node.columns[cols_index].columnToken;
                            primaryKey = node.columns[cols_index].columnToken;
                            i++;
                        }
                    }

                    else if(tokens[i].type == TOKEN_SYMBOL){
                        if(strcmp(tokens[i].value, "=") == 0){
                            if(prevType != TOKEN_IDENTIFIER){
                                printErrorMsg(tokenRet.sql, tokens[i].start, "Invalid column selected before assignment");
                            }
                            node.columns[cols_index].symbol = tokens[i];
                            prevType = TOKEN_SYMBOL;
                        }
                        else if(strcmp(tokens[i].value, ",") == 0){
                            cols_index++;
                            prevType = TOKEN_EMPTY;
                        }
                    }

                    else if(tokens[i].type == TOKEN_STRING || tokens[i].type == TOKEN_NUMBER){
                        if(prevType != TOKEN_SYMBOL){
                            printErrorMsg(tokenRet.sql, tokens[i].start, "Is not a valid string or number assignment");
                        }
                        node.columns[cols_index].valueToken = tokens[i];
                        prevType = tokens[i].type;
                    }
                    i++;
                }
                if(isInsert){
                    i++;
                    if(i + 1 == len - 1 ){
                        printErrorMsg(tokenRet.sql, tokens[i].start, "Values are missing");
                        return createInvalidNode();
                    }
                    size_t valIdx = 0;
                    if(caseInsensitiveCompare(tokens[i].value, "VALUES") == 0){
                        while (i < len){
                            if(tokens[i].type == TOKEN_L_PAR || tokens[i].type == TOKEN_SYMBOL){
                                if(tokens[i].value[0] != ',' && tokens[i].value[0] != '('){
                                    printErrorMsg(tokenRet.sql, tokens[i].start, "Invalid symbol");
                                    return createInvalidNode();
                                }

                            }
                            else if(tokens[i].type == TOKEN_STRING || tokens[i].type == TOKEN_NUMBER || tokens[i].type == TOKEN_BUILT_IN_FUNC){
                                node.columns[valIdx].valueToken = tokens[i];
                                valIdx++;
                            }
                            else if(tokens[i].type == TOKEN_R_PAR){
                                break;
                            }
                            i++;
                        }
                    }
                    if(valIdx != cols_index + 1){
                        printErrorMsg(tokenRet.sql, tokens[i].start, "Values are missing, values doesn't match the number of columns");
                        return createInvalidNode();
                    }
                }
                node.isAllCol = 0;
                node.colsLen = cols_index + 1;
                colsSet = 1;
            }



            // Filter keyword selector
            else if(isFilterKeyword(cur.value)){
                int cols_index = 0;
                i++;
                int start = (int)i;
                Column temp;
                TokenType prevType = TOKEN_EMPTY;
                while(i < len){
                    if(start == i && tokens[i].type != TOKEN_IDENTIFIER){
                        return handleWhereClauseError(tokenRet.sql, tokens[i].start);
                    }
                    if(tokens[i].type == TOKEN_IDENTIFIER){
                        if(prevType != TOKEN_EMPTY){
                            return handleWhereClauseError(tokenRet.sql, tokens[i].start);
                        }
                        node.filters[cols_index].columnToken = tokens[i];
                        prevType = TOKEN_IDENTIFIER;
                    }

                    else if(tokens[i].type == TOKEN_SYMBOL){
                        if(prevType != TOKEN_IDENTIFIER){
                            return handleWhereClauseError(tokenRet.sql, tokens[i].start);
                        }
                        node.filters[cols_index].symbol = tokens[i];
                        prevType = TOKEN_SYMBOL;
                    }

                    else if(tokens[i].type == TOKEN_STRING || tokens[i].type == TOKEN_NUMBER){
                        if(prevType != TOKEN_SYMBOL){
                            return handleWhereClauseError(tokenRet.sql, tokens[i].start);
                        }
                        node.filters[cols_index].valueToken = tokens[i];
                        prevType = tokens[i].type;
                    }

                    else if(tokens[i].type == TOKEN_KEYWORD && isLogicalOperator(tokens[i].value)){
                        if(prevType != TOKEN_NUMBER && prevType != TOKEN_STRING){
                            return handleWhereClauseError(tokenRet.sql, tokens[i].start);
                        }
                        if(isLogicalOperator(tokens[i].value)){
                            node.filters[cols_index].nextLogicalOp = tokens[i];
                            cols_index++;
                            prevType = TOKEN_EMPTY;
                        }
                        else{
                            return handleWhereClauseError(tokenRet.sql, tokens[i].start);
                        }
                    }

                    if(tokens[i].type == TOKEN_STRING && i < len - 2){
                        if(tokens[i+1].type != TOKEN_KEYWORD && isLogicalOperator(tokens[i+1].value)){
                            cols_index++;

                        }
                    }

                    i++;
                }
                node.filtersLen = cols_index + 1;
            }

        }
        i++;
    }
    node.sql = tokenRet.sql;
    return node;
}

NodeList emptyNodeList(){
    NodeList nodeList = {NULL, NULL, 0};
    return nodeList;
};

void insertInNodeList(NodeList *nodeList, Node *node){
    size_t newSize = nodeList->size + 1;
    Node** newNodes = realloc(nodeList->nodes, sizeof(Node*) * newSize);
    char** newTables = realloc(nodeList->tables, sizeof(char*) * newSize);

    if (newNodes != NULL && newTables != NULL) {
        newNodes[nodeList->size] = node;
        newTables[nodeList->size] = node->table.value;
        nodeList->nodes = newNodes;
        nodeList->tables = newTables;
        nodeList->size = newSize;
    } else {
        if (newNodes != NULL) {
            free(newNodes);
        }
        printError("Error creating node of tables");
    }
}

Node *getNodeFromList(NodeList *nodeList, char* table){
    size_t idx = 0;
    for (; idx < nodeList->size; ++idx) {
        if(caseInsensitiveCompare(nodeList->tables[idx], table) == 0){
            return nodeList->nodes[idx];
        }
    }
    return NULL;
}

