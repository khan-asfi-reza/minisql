#ifndef MINISQL_UTILS_H
#define MINISQL_UTILS_H


void removeSingleQuotes(char *str);
int caseInsensitiveCompare(const char *str1, const char *str2) ;

int isKeyword(const char* str) ;
int isDataType(const char* str) ;
int isBuiltInFunc(const char* str) ;
int isPreTableSelectorKeyword(const char* str);
int isNumber(const char *str) ;

int isFilterKeyword(const char* str);
int isLogicalOperator(const char* str);
int isSpecialPunct(char c);

void stringToLower(char *str);
int isSelectKeyword(const char *str);

int isInsertKeyword(const char* str);
int isCreateKeyword(const char* str);
size_t strToLongInt(const char *str);
int isUpdateKeyword(const char* str);
int isDeleteKeyword(const char* str);
int isSymbol(const char *str);
void replaceString(char *str, size_t idx, size_t endIdx, const char *subString);
int isDateType(const char* str);
int isValueFunc(const char* str);
char* concatStrings(const char *strings[], int count);


int isInStringArray(char* array[], size_t  arraySize, char* key);

void printSuccess(const char *format, ...);
void printError(const char *format, ...);

char *createBuffer();
void insertInBuffer(char **buffer, const char *format, ...);
char *createBufferWithSize(size_t size);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* escapeCommas(const char* input);
size_t max(size_t a, size_t b);
void clearBuffer(char **buffer);
#endif
//MINISQL_UTILS_H
