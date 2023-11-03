#include <ctype.h>
#include "const.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

void removeSingleQuotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '\'' && str[len - 1] == '\'') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}


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

int isDataType(const char* str) {
    for (size_t i = 0; i < LEN_DATA_TYPES; i++) {
        if (caseInsensitiveCompare(str, DATA_TYPES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

size_t strToLongInt(const char *str) {
    char *strPtr;
    errno = 0;
    size_t value = strtol(str, &strPtr, 10);
    return value;
}

int isBuiltInFunc(const char* str) {
    for (size_t i = 0; i < LEN_BUILT_IN_FUNC; i++) {
        if (caseInsensitiveCompare(str, BUILT_IN_FUNC[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


int isValueFunc(const char* str) {
    for (size_t i = 0; i < VALUE_FUNC_LEN; i++) {
        if (caseInsensitiveCompare(str, VALUE_FUNC[i]) == 0) {
            return 1;
        }
    }
    return 0;
}



int isKeyword(const char* str) {
    for (size_t i = 0; i < LEN_KEYWORDS; i++) {
        if (caseInsensitiveCompare(str, KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isPreTableSelectorKeyword(const char* str) {
    for (size_t i = 0; i < LEN_PRE_TB_SELECTOR_KEYWORDS; i++) {
        if (caseInsensitiveCompare(str, PRE_TB_SELECTOR_KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


int isLogicalOperator(const char* str) {
    for (size_t i = 0; i < LEN_LOGICAL_OP; i++) {
        if (caseInsensitiveCompare(str, LOGICAL_OP[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


int isDateType(const char* str) {
    for (size_t i = 0; i < LEN_LOGICAL_OP; i++) {
        if (caseInsensitiveCompare(str, DATE_TYPES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}



int isFilterKeyword(const char* str){
    return caseInsensitiveCompare(str, FILTER_KEYWORD) == 0;
}


int isInStringArray(char**array, size_t  arraySize, char* key){
    if (key == NULL) {
        return 0;
    }

    if(arraySize == -1){
        return 0;
    }

    if (array == NULL) {
        return 0;
    }
    for (size_t i = 0; i <= arraySize; i++) {
        if (strcmp(key, array[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


char* concatStrings(const char *strings[], int count) {

    size_t totalLength = 1;
    for (int i = 0; i < count; i++) {
        totalLength += strlen(strings[i]);
    }

    char *result = malloc(totalLength * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(result, strings[i]);
    }

    return result;
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

int isSpecialPunct(char c){
    return c != '\'' && ispunct(c);
}

int isSymbol(const char *str){
    if(strcmp(str, ">=") == 0 || strcmp(str, "<=") == 0 || strcmp(str, "!=") == 0){
        return 1;
    }
    return isSpecialPunct(str[0]);
}

void replaceString(char *str, size_t idx, size_t endIdx, const char *subString) {
    size_t len = strlen(str);
    if (idx >= len) {
        return;
    }
    size_t newLen = strlen(subString);
    size_t ogSubLen = endIdx - idx + 1;
    size_t newTotalLen = len - ogSubLen + newLen;
    char *newStr = realloc(str, newTotalLen + 1);
    if (newStr == NULL) {
        return;
    }
    str = newStr;
    memmove(str + idx + newLen, str + idx + ogSubLen, len - idx - ogSubLen + 1);
    memcpy(str + idx, subString, newLen);
}


void stringToLower(char* str){
    if (str == NULL) return; // safety check

    for (int i = 0; str[i]; i++) {
        str[i] = (char) tolower((unsigned char) str[i]);
    }
}

int isSelectKeyword(const char* str){
    return caseInsensitiveCompare(str, SELECT_KEYWORD) == 0;
}

int isInsertKeyword(const char* str){
    return caseInsensitiveCompare(str, "INSERT") == 0;
}


int isCreateKeyword(const char* str){
    return caseInsensitiveCompare(str, "CREATE") == 0;
}

int isUpdateKeyword(const char* str){
    return caseInsensitiveCompare(str, "UPDATE") == 0;
}

int isDeleteKeyword(const char* str){
    return caseInsensitiveCompare(str, "DELETE") == 0;
}



void printError(const char *format, ...){
    va_list args;
    va_start(args, format);
    printf("\033[1;31m");
    vprintf(format, args);
    printf("\033[0m\n");
    va_end(args);
}

void printSuccess(const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("\033[32m");
    vprintf(format, args);
    printf("\033[0m\n");
    va_end(args);
}

char *createBuffer() {
    char *buffer = malloc(1);
    if (buffer == NULL) {
        perror("Memory reallocation failed for string buffer");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0';
    return buffer;
}

char *createBufferWithSize(size_t size){
    char *buffer = malloc(size + 1);
    if (buffer == NULL) {
        perror("Memory reallocation failed for string buffer");
        exit(EXIT_FAILURE);
    }
    buffer[size + 1] = '\0';
    return buffer;
}

void insertInBuffer(char **buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int neededSize = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    if (neededSize <= 0) {
        fprintf(stderr, "Formatting error\n");
        return;
    }
    char *newBuffer = realloc(*buffer, strlen(*buffer) + neededSize);
    if (newBuffer == NULL) {
        perror("Memory reallocation failed for string buffer");
        exit(EXIT_FAILURE);
    }
    *buffer = newBuffer;
    va_start(args, format);
    vsprintf(*buffer + strlen(*buffer), format, args);
    va_end(args);
}

void clearBuffer(char **buffer) {
    free(*buffer);
    *buffer = NULL;
}

size_t max(size_t a, size_t b){
    return a > b ? a : b;
};

char* escapeCommas(const char* input) {
    int cCount = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ',') {
            cCount++;
        }
    }
    char* result = malloc(strlen(input) + cCount + 1);
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ',') {
            result[j++] = '\\';
        }
        result[j++] = input[i];
    }
    result[j] = '\0';
    return result;
}