//
// Created by Khan Asfi Reza on 27/10/23.
//
#include "stdlib.h"
#include "lexer.h"

#ifndef MINISQL_DB_H
#define MINISQL_DB_H

#define MAX_COL_SIZE 25

int getColumnIndex(Node* node, char* column);
int matchColumnValue(char* table, size_t colIdx, char* str);

NodeList loadTables();

struct {
    Column columns[COL_MAX_SIZE];
    int size;
}typedef TempRow;

typedef enum {
        SUCCESS,
        INTERNAL_ERROR,
        FAIL
} DBCode;

size_t getMaxColSize(size_t a, size_t b);

struct {
    char* result;
    char* error;
    char* successMsg;
    size_t maxColSpace;
    DBCode code ; // 0 - Internal db error , 1 - Success, 4 - DB User error
    size_t lineCount;
    int colCount;
} typedef DBOp ; // DB Operation Return type


DBOp createDBOp();
DBOp createDbOpWithHeader(Node sqlNode, Node tableNode);

DBOp dbCreateTable(Node node);
DBOp dbInsert(Node sqlNode, Node tableNode);
DBOp dbSelect(Node sqlNode, Node tableNode);
DBOp dbUpdate(Node sqlNode, Node tableNode);
DBOp dbDelete(Node sqlNode, Node tableNode);

void mergeDbOp(DBOp* source, DBOp* destination);

void clearDBOp(DBOp *dbOp);
void printDbOp(DBOp *dbOp);

#endif //MINISQL_DB_H
