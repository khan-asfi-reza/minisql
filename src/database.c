#include "lexer.h"
#include "utils.h"
#include "const.h"
#include "filesystem.h"
#include "database.h"
#include <time.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Identify the maximum space a column is taking while printing the table
 * @param a
 * @param b
 * @return Max column size
 */
size_t getMaxColSize(size_t a, size_t b){
    size_t c = max(a,b);
    // Checking for even if the number is even add one to adjust space
    if(isEven(a)){
        c = c+1;
    }
    // Minimum space by a column in the input
    if(c < MIN_COL_SIZE){
        return MIN_COL_SIZE;
    }
    // Maximum space a column can take in the output
    if(c > MAX_COL_SIZE){
        return MAX_COL_SIZE;
    }
    return c;
}

/**
 * Data file is where the data stored for a table
 * Data file's name format "DATA_DIRECTORY/table_(table_name)"
 * @param node SQL AST Node
 * @return name of the data file
 */
char* getTableDataFileName(Node node){
    char* buffer = createBuffer();
    insertInBuffer(&buffer, "%s/table_%s", DATA_DIR, node.table.value);
    return buffer;
}

/**
 * The create table sql is stored to get the reference of the table,
 * To identify the table's column and index field.
 * Data file's name format "DATA_DIRECTORY/table_(table_name)_sql"
 * @param node SQL AST Node
 * @return name of the data sql file
 */
char* getTableSQLName(Node node){
    char* buffer = createBuffer();
    insertInBuffer(&buffer, "%s/table_%s_sql", DATA_DIR, node.table.value);
    return buffer;
}


/**
 * The Primary key number is stored to get the last used id/serial of the table,
 * Data file's name format "DATA_DIRECTORY/table_(table_name)_pk"
 * @param node SQL AST Node
 * @return name of the data file
 */
char* getTablePkName(Node node){
    char* buffer = createBuffer();
    insertInBuffer(&buffer, "%s/table_%s_pk", DATA_DIR, node.table.value);
    return buffer;
}

/**
 * Storage to store all the table sql file name
 * @return name of the data file
 */
char* getTableConfFileName(){
    char* buffer = createBuffer();
    insertInBuffer(&buffer, "%s/.table", DATA_DIR);
    return buffer;
}


/**
 * Creates an empty db operation object
 * @return Empty db operation
 */
DBOp createDBOp(){
    DBOp dbOperation;
    dbOperation.code = SUCCESS;
    dbOperation.successMsg = createBuffer();
    dbOperation.error = createBuffer();
    dbOperation.result = createBuffer();
    dbOperation.action = createBuffer();
    dbOperation.rows = malloc(sizeof(char ) * 1);
    dbOperation.rowCount = 0;
    dbOperation.maxColSpace = 5;
    dbOperation.lineCount = 0;
    dbOperation.colCount = 0;
    return dbOperation;
}

/**
 * Frees allocated memory for a db operation
 * @param dbOp db operation object
 */
void clearDBOp(DBOp *dbOp){
    if(dbOp->error != NULL){
        free(dbOp->error);
    }
    if(dbOp->result != NULL){
        free(dbOp->result);
    }
    if(dbOp->successMsg != NULL){
        free(dbOp->successMsg);
    }
    if(dbOp->action != NULL){
        free(dbOp->action);
    }
}


/**
 * Creates default value by a default function
 * @return Default value
 */
char* defaultValue(Token token){
    // Default function = NOW
    // Generates current time while creation of the data
    if(caseInsensitiveCompare(token.value, "NOW") == 0){
        // Allocate memory for current timestamp
        time_t now;
        struct tm *utc;
        char *dateTimeStr;
        dateTimeStr = createBufferWithSize(sizeof(char) * 100);
        if (dateTimeStr == NULL) {
            printError("Memory allocation failed while creating timestamp.\n");
            return "";
        }
        time(&now);
        // Get utc time from now time
        utc = gmtime(&now);
        // Format the time
        if (strftime(dateTimeStr, 100, "%Y-%m-%d %H:%M:%S GMT+0", utc) == 0) {
            printError("Failed to format date-time string.\n");
            free(dateTimeStr);
            return "";
        }
        return dateTimeStr;
    }
    return "1";
}

/**
 * Creates a table
 * @return DBOp
 */
DBOp dbCreateTable(Node sqlNode){

    DBOp dbOperation = createDBOp();
    FILE *tableFile = NULL, *tableSqlFile = NULL, *tableConfig = NULL;
    char* tableFullName = getTableDataFileName(sqlNode);
    char* tableSql = getTableSQLName(sqlNode);
    char* tableConfStr = getTableConfFileName();
    tableConfig = fopen(tableConfStr, "a");
    char *pKeyFile;
    if(fileExists(tableFullName) || fileExists(tableSql)){
        insertInBuffer(&dbOperation.error, "Table `%s` already exists", sqlNode.table.value);
        dbOperation.code = FAIL;
        return dbOperation;
    }
    if(dbOperation.code == SUCCESS){
        if(tableConfig == NULL){
            insertInBuffer(&dbOperation.error, "Database Corrupted", sqlNode.table.value);
            dbOperation.code = INTERNAL_ERROR;
            return dbOperation;
        }
        else{
            fputs(tableSql, tableConfig);
            fputs("\n", tableConfig);
            fclose(tableConfig);
        }
    }
    if(dbOperation.code == SUCCESS){
        tableFile = fopen(tableFullName, "a");
        tableSqlFile = fopen(tableSql, "a+");
        if(tableFile != NULL && tableSqlFile != NULL){
            for (int i = 0; i < sqlNode.colsLen; ++i) {
                if(caseInsensitiveCompare(sqlNode.columns[i].columnToken.value, "id") == 0){
                    pKeyFile = getTablePkName(sqlNode);
                    FILE *file = fopen(pKeyFile, "w");
                    fprintf(file, "1");
                    fclose(file);
                    free(pKeyFile);
                }
            }
            fprintf(tableSqlFile, "%s", sqlNode.sql);
            fclose(tableSqlFile);
            fclose(tableFile);
            insertInBuffer(&dbOperation.successMsg, "Created table `%s`", sqlNode.table.value);
        }
        else{
            insertInBuffer(&dbOperation.error, "Error creating table, table data is corrupted\n");
            dbOperation.code = INTERNAL_ERROR;
        }
    }
    free(tableFullName);
    free(tableSql);
    free(tableConfStr);
    return dbOperation;
}


int getColumnIndex(Node* node, char* column){
    for (int i = 0; i < node->colsLen ; i++) {
        if(caseInsensitiveCompare(node->columns[i].columnToken.value, column) == 0){
            return i;
        }
    }
    return -1;
}

int getFilterIndex(Node* node, char* column){
    for (int i = 0; i < node->filtersLen ; i++) {
        if(caseInsensitiveCompare(node->filters[i].columnToken.value, column) == 0){
            return i;
        }
    }
    return -1;
}


int matchColumnValue(char* table, size_t colIdx, char* str){
    FILE *file;
    file = fopen(table, "r");
    char *line = NULL;
    size_t len = 0;
    if (file == NULL) {
        printError("Database Table `%s` corrupted", table);
        return -1;
    }
    int lineCount = 0;
    while (getLine(&line, &len, file) != -1) {
        int start = 0;
        int i = 0;
        size_t commas = 0;
        while (1){
            if(line[i] == ','){
                commas++;
                if(commas == colIdx + 2){
                    char value[i - start + 1];
                    strncpy(value, line + start, i - start);
                    value[i - start] = '\0';
                    if(strcmp(value, str) == 0){
                        return 1;
                    }
                    else{
                        break;
                    }
                }
                else{
                    start = i + 1;
                }
            }
            i++;
        }
        lineCount++;
    }
    free(line);
    fclose(file);
    return 0;
}


NodeList loadTables(){
    FILE *file;
    NodeList nodeList = emptyNodeList();
    const char* tableConf[] = {DATA_DIR, "/", ".table"};
    char* tableConfStr = concatStrings(tableConf, 3);
    file = fopen(tableConfStr, "r");
    char *line = NULL;
    size_t len = 0;
    if (file == NULL) {
        printError("Database corrupted");
        return nodeList;
    }
    size_t size = 0;
    while ((getLine(&line, &len, file)) != -1) {
        size_t s_len = strlen(line);
        line[s_len-1] = '\0';
        FILE *sqlFile = fopen(line, "r");
        size_t internalLen = 0;
        if(sqlFile != NULL){
            size++;
            char *sql = NULL;
            getLine(&sql, &internalLen, sqlFile);
            TokenRet tokenRet = lexAnalyze(sql);
            Node node = createASTNode(tokenRet);
            Node *newNode = malloc(sizeof(node) * 1);
            *newNode = node;
            insertInNodeList(&nodeList, newNode);
            free(sql);
        }
    }
    nodeList.size = size;
    free(line);
    fclose(file);
    return nodeList;
}

DBOp createDbOpWithHeader(Node sqlNode, Node tableNode){
    DBOp header = createDBOp();
    Node sNode = sqlNode;
    if(sqlNode.isAllCol || isInsertKeyword(sqlNode.action.value)){
        sNode = tableNode;
    }

    int i = 0;
    for (; i < sNode.colsLen; ++i) {
        int col_idx = getColumnIndex(&tableNode, sNode.columns[i].columnToken.value);
        if(col_idx == -1){
            printError(
                    "Invalid column `%s`, column `%s` doesn't exist in table",
                    sNode.columns[i].columnToken.value,
                    sNode.columns[i].columnToken.value,
                    sNode.table.value
            );
            clearBuffer(&header.error);
            header.code = FAIL;
            return header;
        }
        else if(col_idx < COL_MAX_SIZE){
            header.maxColSpace = getMaxColSize(header.maxColSpace, strlen(tableNode.columns[col_idx].columnToken.value));
            insertInBuffer(&header.result, "%s", tableNode.columns[col_idx].columnToken.value);
        }
        if(i != sNode.colsLen - 1){
            insertInBuffer(&header.result, ",");
        }
        else{
            insertInBuffer(&header.result, "\n");
        }
    }
    header.lineCount = 1;
    header.colCount = i;
    header.action = createBufferWithSize(strlen(sNode.action.value));
    insertInBuffer(&header.action, "%s", sqlNode.action.value);
    return header;
}




size_t getPkFromPkFile(FILE *pkFile){
    if(pkFile != NULL){
        size_t _id;
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), pkFile) != NULL) {
            char *endPointer;
            long int temp_id = strtol(buffer, &endPointer, 10);
            if (*endPointer == '\0' || *endPointer == '\n') {
                _id = (size_t)temp_id;
                return _id;
            } else {
                printError("Invalid pk file, pk file is corrupted\n");
                fprintf(stderr, "Conversion error occurred\n");
            }
        } else {
            printError("Invalid pk file, pk file is corrupted\n");
            fprintf(stderr, "Read error occurred\n");
        }
    }
    return -1;
}


int deleteLine(const char *filename, const size_t *lines, size_t size) {
    FILE *file;
    char *buffer, *newBuffer;
    long length;
    size_t line_count = 1;
    size_t current_line = 0;
    char *cursor, *next_line;
    file = fopen(filename, "r");
    if (file == NULL) {
        printError("Unable to open file for deletion");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file);

    buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        printError("Error allocating memory");
        fclose(file);
        return -1;
    }

    fread(buffer, length, 1, file);
    fclose(file);
    buffer[length] = '\0';

    newBuffer = (char *)malloc(length + 1);
    if (newBuffer == NULL) {
        printError("Error allocating memory for new buffer");
        free(buffer);
        return -1;
    }

    cursor = buffer;
    next_line = NULL;
    size_t newBufferIdx = 0;
    while (line_count <= lines[size - 1] && *cursor != '\0') {
        if (current_line < size && line_count == lines[current_line]) {
            next_line = strchr(cursor, '\n');
            if (next_line != NULL) {
                cursor = next_line + 1;
            } else {
                cursor = cursor + strlen(cursor);
            }
            current_line++;
        } else {
            next_line = strchr(cursor, '\n');
            size_t line_length = (next_line != NULL) ? (size_t)(next_line - cursor) + 1 : strlen(cursor);
            memcpy(newBuffer + newBufferIdx, cursor, line_length);
            newBufferIdx += line_length;
            cursor += line_length;
        }
        line_count++;
    }
    if (*cursor != '\0') {
        strcpy(newBuffer + newBufferIdx, cursor);
    }
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        free(buffer);
        free(newBuffer);
        return -1;
    }
    fwrite(newBuffer, strlen(newBuffer), 1, file);
    fclose(file);
    free(buffer);
    free(newBuffer);
    return 0;
}


int filterValue(Column filter, const char* value){
    int shouldInsertInRow = 1;
    removeSingleQuotes(filter.valueToken.value);
    if(filter.valueToken.type == TOKEN_NUMBER && isNumber(value)){
        size_t val = strToLongInt(value);
        size_t cmpVal = strToLongInt(filter.valueToken.value);
        if(strcmp(filter.symbol.value, "<") == 0){
            if(val < cmpVal){
                shouldInsertInRow = 1;
            }
            else{
                shouldInsertInRow = 0;
            }
        }
        else if(strcmp(filter.symbol.value, ">") == 0){
            if(val > cmpVal){
                shouldInsertInRow = 1;
            }
            else{
                shouldInsertInRow = 0;
            }
        }
        else if(strcmp(filter.symbol.value, ">=") == 0){
            if(val >= cmpVal){
                shouldInsertInRow = 1;
            }
            else{
                shouldInsertInRow = 0;
            }
        }
        else if(strcmp(filter.symbol.value, "<=") == 0){
            if(val <= cmpVal){
                shouldInsertInRow = 1;
            }
            else{
                shouldInsertInRow = 0;
            }
        }
        else if(strcmp(filter.symbol.value, "!=") == 0){
            if(val != cmpVal){
                shouldInsertInRow = 1;
            }
            else{
                shouldInsertInRow = 0;
            }
        }
        else if(strcmp(filter.symbol.value, "=") == 0){
            if(val == cmpVal){
                shouldInsertInRow = 1;
            }
            else{
                shouldInsertInRow = 0;
            }
        }
        else{
            shouldInsertInRow = 0;
        }
    }
    else if(strcmp(filter.valueToken.value, value) != 0){
        shouldInsertInRow = 0;
    }
    return shouldInsertInRow;
}

DBOp dbUpdate(Node sqlNode, Node tableNode){
    Node sNode = sqlNode;
    if(sqlNode.isAllCol){
        sNode = tableNode;
    }
    DBOp dbOp = createDbOpWithHeader(sqlNode, tableNode);

    size_t upCount = 0;
    char *tableName = getTableDataFileName(sqlNode);
    char* tBuffer = createBuffer();
    FILE *tableFile = fopen(tableName, "r+");
    char **rows = malloc(sizeof(char ) * 1);
    size_t rowCount = 0;
    if(tableFile != NULL){
        char *line = NULL;
        size_t len = 0;
        int lineCount = 0;
        while (getLine(&line, &len, tableFile) != -1){
            lineCount++;
            int shouldInsertInRow = 1;
            for (int fil = 0; fil < sNode.filtersLen; ++fil) {
                Column filter = sNode.filters[fil];
                int colIdx = getColumnIndex(&tableNode, sNode.filters[fil].columnToken.value);
                size_t start = 0;
                size_t i = 0;
                size_t commas = 0;
                while (1){
                    if((i>0 && line[i] == ',' && line[i-1] != '\\') || line[i] == '\n'){
                        commas++;
                        if(colIdx == commas - 2){
                            char *value = createBufferWithSize(i - start);
                            strncpy(value, line + start, i - start);
                            value[i - start] = '\0';
                            removeSingleQuotes(filter.valueToken.value);
                            shouldInsertInRow = filterValue(filter, value);
                            clearBuffer(&value);
                            break;
                        }
                        else{
                            start = i + 1;
                        }
                    }
                    i++;
                }
                if(shouldInsertInRow == 0){
                    break;
                }
            }
            if(shouldInsertInRow == 1){
                upCount++;
                for (int col = 0; col < sNode.colsLen; ++col) {
                    Column column = sNode.columns[col];
                    int colIdx = getColumnIndex(&tableNode, column.columnToken.value);
                    size_t iStart = 0;
                    size_t j = 0;
                    size_t commas = 0;
                    while (1){
                        if((j>0 && line[j] == ',' && line[j-1] != '\\') || line[j] == '\n'){
                            commas++;
                            if(colIdx == commas - 2){
                                int match = 0;
                                if(tableNode.columns[colIdx].isUnique == 1){
                                    match = matchColumnValue(tableName, colIdx, column.valueToken.value);
                                }
                                if(match == 0 && upCount < 2){
                                    removeSingleQuotes(column.valueToken.value);
                                    replaceString(line, iStart, j-1, column.valueToken.value);
                                    break;
                                }
                                else{
                                    dbOp.code = FAIL;
                                    insertInBuffer(&dbOp.error, "Duplicate value `%s` for column `%s` violates unique constraint", column.valueToken.value, column.columnToken.value);
                                    free(tableName);
                                    free(tBuffer);
                                    free(line);
                                    return dbOp;
                                }
                            }

                            else{
                                iStart = j + 1;
                            }
                        }
                        j++;
                    }
                }
                rows[rowCount] = createBuffer();
                insertInBuffer(&rows[rowCount], line);
                rowCount++;
                char **tempRow = realloc(rows, sizeof(char) * rowCount);
                if(tempRow != NULL){
                    rows = tempRow;
                }
                else{
                    dbOp.code = FAIL;
                    insertInBuffer(&dbOp.error, "MEM Failed");
                    free(tableName);
                    free(tBuffer);
                    free(line);
                    return dbOp;
                }

            }
            insertInBuffer(&tBuffer, "%s", line);
        }
        fclose(tableFile);
        dbOp.lineCount += lineCount;
    }
    dbOp.rows = rows;
    dbOp.rowCount = rowCount;
    FILE *writeFile = fopen(tableName, "w");
    if(writeFile != NULL){
        fprintf(writeFile, "%s", tBuffer);
    }
    fclose(writeFile);
    clearBuffer(&tBuffer);
    insertInBuffer(&dbOp.successMsg, "Updated `%zd` rows in table %s", upCount, sNode.table.value);
    free(tableName);
    return dbOp;
}


DBOp dbSelect(Node sqlNode, Node tableNode){
    Node sNode = sqlNode;
    if(sqlNode.isAllCol){
        sNode = tableNode;
    }
    DBOp dbOp = createDbOpWithHeader(sqlNode, tableNode);
    char *tableName = getTableDataFileName(sqlNode);
    FILE *tableFile = fopen(tableName, "r");
    char **rows = malloc(sizeof(char ) * 1);
    size_t rowCount = 0;

    if(tableFile != NULL){
        char *line = NULL;
        size_t len = 0;
        int lineCount = 0;
        while (getLine(&line, &len, tableFile) != -1){
            lineCount++;
            char* rowBuffer = createBuffer();
            int shouldInsertInRow = 1;
            for (size_t fil = 0; fil < sqlNode.filtersLen; ++fil) {
                size_t start = 0;
                size_t i = 0;
                size_t commas = 0;
                int col_idx = getColumnIndex(&tableNode, sqlNode.filters[fil].columnToken.value);
                while (1){
                    if((i>0 && line[i] == ',' && line[i-1] != '\\') || line[i] == '\n'){
                        commas++;
                        if(commas - 2 == col_idx){
                            char *value = createBufferWithSize(i - start);
                            strncpy(value, line + start, i - start);
                            shouldInsertInRow = filterValue(sqlNode.filters[fil], value);
                            free(value);
                            break;
                        }
                        else{
                            start = i + 1;
                        }
                        if(line[i] == '\n'){
                            break;
                        }
                    }
                    i++;
                }
                if(sqlNode.filters[fil].nextLogicalOp.value != NULL && caseInsensitiveCompare(sqlNode.filters[fil].nextLogicalOp.value, "AND") == 0){
                    if(shouldInsertInRow == 0){
                        break;
                    }
                }else{
                    if(shouldInsertInRow == 1){
                        break;
                    }
                }
            }
            if(shouldInsertInRow == 1){
                for (int col = 0; col < sNode.colsLen; ++col) {
                    int col_idx = getColumnIndex(&tableNode, sNode.columns[col].columnToken.value);
                    size_t start = 0;
                    size_t i = 0;
                    size_t commas = 0;
                    while (1){
                        if((i>0 && line[i] == ',' && line[i-1] != '\\') || line[i] == '\n'){
                            commas++;
                            char *value = createBufferWithSize(i - start);
                            strncpy(value, line + start, i - start);
                            value[i - start] = '\0';
                            if(commas == col_idx + 2){
                                insertInBuffer(&rowBuffer, "%s", value);
                                dbOp.maxColSpace = getMaxColSize(dbOp.maxColSpace, strlen(value));
                                clearBuffer(&value);
                            }
                            else{
                                start = i + 1;
                            }
                            clearBuffer(&value);
                            if(line[i] == '\n'){
                                break;
                            }
                        }
                        i++;
                    }
                    if(col != sNode.colsLen - 1){
                        insertInBuffer(&rowBuffer, ",");
                    }
                }
                insertInBuffer(&rowBuffer, "\n");
                insertInBuffer(&dbOp.result, "%s", rowBuffer);
                rows[rowCount] = createBuffer();
                insertInBuffer(&rows[rowCount], line);
                rowCount++;
                char **tempRow = realloc(rows, sizeof(char) * rowCount);
                if(tempRow != NULL){
                    rows = tempRow;
                }
                else{
                    dbOp.code = FAIL;
                    insertInBuffer(&dbOp.error, "MEM Failed");
                    free(tableName);
                    free(line);
                    return dbOp;
                }
                clearBuffer(&rowBuffer);
            }
        }
        fclose(tableFile);
        dbOp.lineCount += lineCount;
    }
    dbOp.rows = rows;
    dbOp.rowCount = rowCount;
    return dbOp;
}

DBOp dbDelete(Node sqlNode, Node tableNode){
    Node sNode = sqlNode;
    if(sqlNode.isAllCol){
        sNode = tableNode;
    }
    DBOp dbOp = createDbOpWithHeader(sqlNode, tableNode);
    size_t *linesToDelete = malloc(sizeof(size_t) * 1);
    size_t lIdx = 0;
    char *tableName = getTableDataFileName(sqlNode);
    FILE *tableFile = fopen(tableName, "r");
    if(tableFile != NULL){
        char *line = NULL;
        size_t len = 0;
        int lineCount = 0;
        while (getLine(&line, &len, tableFile) != -1){
            lineCount++;
            int shouldInsertInRow = 1;
            for (int fil = 0; fil < sNode.filtersLen; ++fil) {
                Column filter = sNode.filters[fil];
                int colIdx = getColumnIndex(&tableNode, sNode.filters[fil].columnToken.value);
                size_t start = 0;
                size_t i = 0;
                size_t commas = 0;
                while (1){
                    if((i>0 && line[i] == ',' && line[i-1] != '\\') || line[i] == '\n'){
                        commas++;
                        if(colIdx == commas - 2){
                            char *value = createBufferWithSize(i - start);
                            strncpy(value, line + start, i - start);
                            value[i - start] = '\0';
                            removeSingleQuotes(filter.valueToken.value);
                            shouldInsertInRow = filterValue(filter, value);
                            clearBuffer(&value);
                            break;
                        }
                        else{
                            start = i + 1;
                        }
                    }
                    i++;
                }
                if(shouldInsertInRow == 0){
                    break;
                }
            }
            if(shouldInsertInRow == 1){
                linesToDelete[lIdx] = lineCount;
                lIdx++;
                size_t *nL = realloc(linesToDelete, sizeof(size_t)*lIdx);
                if(nL != NULL){
                    linesToDelete=nL;
                }
                else{
                    return dbOp;
                }
            }

        }
        fclose(tableFile);
        dbOp.lineCount += lineCount;
    }
    int r = deleteLine(tableName, linesToDelete, lIdx);
    if(r == 0){
        insertInBuffer(&dbOp.successMsg, "Deleted `%zd` rows in table %s", lIdx, sNode.table.value);
    }
    else{
        insertInBuffer(&dbOp.error, "Unable to delete row in table `%s`", tableName);
    }
    free(linesToDelete);
    free(tableName);
    return dbOp;
}


DBOp dbInsert(Node sqlNode, Node tableNode){
    DBOp dbOp = createDbOpWithHeader(sqlNode, tableNode);
    char* tableName = getTableDataFileName(sqlNode);
    if(dbOp.code != SUCCESS){
        return dbOp;
    }
    size_t _id = -1;
    FILE *pkFile;
    char **rows = malloc(sizeof(char) * 1);
    size_t rowCount = 0;
    if(fileExists(tableName)){
        FILE *table = fopen(tableName, "a+");
        if(table != NULL){
            char* rowBuffer = createBuffer();
            insertInBuffer(&rowBuffer, "1,");
            for (int i = 0; i < tableNode.colsLen; ++i) {
                int col_idx = getColumnIndex(&sqlNode, tableNode.columns[i].columnToken.value);
                if(caseInsensitiveCompare(tableNode.columns[i].columnToken.value, "id") == 0){
                    char *pkFileName = getTablePkName(sqlNode);
                    pkFile = fopen(pkFileName, "r+");
                    _id = getPkFromPkFile(pkFile);
                    _id++;
                    if (_id != -1) {
                        insertInBuffer(&dbOp.result, "%zd", _id);
                        insertInBuffer(&rowBuffer, "%zd", _id);
                    }
                    else{
                        if(col_idx != - 1){
                            insertInBuffer(&rowBuffer, "%s", sqlNode.columns[col_idx].valueToken.value);
                            insertInBuffer(&dbOp.result, "%s", sqlNode.columns[col_idx].valueToken.value);
                        }
                    }
                }
                else{
                    if(col_idx >= COL_MAX_SIZE){
                        dbOp.code = FAIL;
                        insertInBuffer(&dbOp.error,
                                       "Insertion failed for table `%s` surpassed the column size %d",
                                       tableName,
                                       COL_MAX_SIZE
                                       );
                        break;
                    }
                    else if(col_idx > -1){
                        removeSingleQuotes(sqlNode.columns[col_idx].valueToken.value);
                        if(tableNode.columns[i].isUnique == 1){
                            int match = matchColumnValue(tableName, i, sqlNode.columns[col_idx].valueToken.value);
                            if(match == 1){
                                fclose(table);
                                insertInBuffer(
                                        &dbOp.error,
                                        "Duplicate value `%s` violates unique constraint on column `%s` for table `%s`;",
                                        sqlNode.columns[col_idx].valueToken.value,
                                        sqlNode.columns[col_idx].columnToken.value,
                                        tableNode.table.value
                                );
                                dbOp.code = FAIL;
                                break;
                            }
                        }
                        insertInBuffer(&rowBuffer, "%s", sqlNode.columns[col_idx].valueToken.value);
                        insertInBuffer(&dbOp.result, "%s", sqlNode.columns[col_idx].valueToken.value);
                    }
                    else{
                        if(tableNode.columns[i].defaultToken.type == TOKEN_BUILT_IN_FUNC){
                            char* val = defaultValue(tableNode.columns[i].defaultToken);
                            insertInBuffer(&dbOp.result, "%s", val);
                            insertInBuffer(&rowBuffer, "%s",val);
                            free(val);
                        }
                    }
                }
                if(i != tableNode.colsLen - 1){
                    insertInBuffer(&rowBuffer, ",");
                    insertInBuffer(&dbOp.result, ",");
                }
            }
            insertInBuffer(&rowBuffer, "\n");
            if(dbOp.code == SUCCESS){
                insertInBuffer(&dbOp.result, "\n");
                rows[rowCount] = createBuffer();
                insertInBuffer(&rows[rowCount], "%s", rowBuffer);
                rowCount++;
                char **tempRow = realloc(rows, sizeof(char) * rowCount);
                if(tempRow != NULL){
                    rows = tempRow;
                }
                else{
                    dbOp.code = FAIL;
                    insertInBuffer(&dbOp.error, "MEM Failed");
                    free(tableName);
                    return dbOp;
                }
                fputs(rowBuffer, table);
            }
            clearBuffer(&rowBuffer);
            fclose(table);

        }
        else{
            dbOp.code = INTERNAL_ERROR;
            insertInBuffer(&dbOp.successMsg, "Insertion failed for table `%s`", tableNode.table.value);
        }
    }
    else{
        dbOp.code = INTERNAL_ERROR;
        insertInBuffer(&dbOp.error, "Table `%s` doesn't exist", tableNode.table.value);
    }
    if(dbOp.code == SUCCESS){
        insertInBuffer(&dbOp.successMsg, "Created record in table `%s`", tableNode.table.value);
        dbOp.lineCount++;
        if(pkFile != NULL){
            fseek(pkFile, 0, SEEK_SET);
            fprintf(pkFile, "%zd", _id);
            fclose(pkFile);
        }
    }
    else if(dbOp.code == FAIL){

    }
    dbOp.rows = rows;
    dbOp.rowCount = rowCount;
    free(tableName);
    return dbOp;
}

void printTables(NodeList nodeList){
    for (int i = 0; i < MAX_COL_SIZE; ++i) {
        printf("_");
    }
    printf("\n");
    printf(" TABLES ");

    printf("\n");
    for (int i = 0; i < MAX_COL_SIZE; ++i) {
        printf("_");
    }
    printf("\n");
    for (size_t i = 0; i < nodeList.size; ++i) {
        printf("%zd |  ", i);
        printf("%s", nodeList.nodes[i]->table.value);
        printf("\n");
        for (size_t j = 0; j < MAX_COL_SIZE; ++j) {
            printf("_");
        }
        printf("\n");
    }
}


DBOp execSQL(char* input, NodeList *tableList){
    TokenRet tokenRet = lexAnalyze(input);
    Node node = createASTNode(tokenRet);
    if(node.isInvalid == 0){
        Node *tableNode = getNodeFromList(tableList, node.table.value);
        if(tableNode != NULL){
            if(isSelectKeyword(node.action.value)){
                DBOp dbOp = dbSelect(node, *tableNode);
                return dbOp;
            }
            else if(isInsertKeyword(node.action.value)){
                DBOp dbOp = dbInsert(node, *tableNode);
                return dbOp;
            }
            else if(isDeleteKeyword(node.action.value)){
                DBOp dbOp = dbDelete(node, *tableNode);
                return dbOp;
            }
            else if(isUpdateKeyword(node.action.value)){
                DBOp dbOp = dbUpdate(node, *tableNode);
                return dbOp;
            }
            else if(isCreateKeyword(node.action.value)){
                printf("Info: Table `%s` exists", node.table.value);
            }
        }
        else{
            if(isCreateKeyword(node.action.value)){
                DBOp dbOp = dbCreateTable(node);
                *tableList = loadTables();
                return dbOp;
            }
            else{
                printError("Table `%s` doesn't exist", node.table.value);
            }
        }

    }
    return createDBOp();
}


void printDbOp(DBOp *dbOp){
    char* result = dbOp->result;
    size_t start = 0;
    size_t mSpace = dbOp->maxColSpace;
    int isEvenTOff;
    for (int i = 0; i < dbOp->colCount * mSpace + dbOp->colCount; ++i) {
        printf("_");
    }
    printf("\n");
    for (int i = 0; i < strlen(result); ++i) {
        if((i>0 && result[i] == ',' && result[i-1] != '\\') || result[i] == '\n'){
            isEvenTOff = 0;
            size_t t_off = (i - start);
            if(t_off % 2 == 0){
                t_off+=1;
                isEvenTOff = 1;
            }
            size_t offset;
            if(t_off > mSpace) {
                offset = 0;
            } else {
                offset = mSpace - t_off;
            }
            for (int j = 0; j < offset/2; ++j) {
                printf(" ");
            }
            if(isEvenTOff == 1){
                printf(" ");
            }
            for (size_t l = start; l < i; ++l) {
                printf("%c", result[l]);
            }
            for (int j = 0; j < offset/2; ++j) {
                printf(" ");
            }
            printf("|");
            if(result[i] == '\n'){
                printf("\n");
                for (int x = 0; x < dbOp->colCount * mSpace + dbOp->colCount; ++x) {
                    printf("-");
                }
                printf("\n");
            }
            start = i + 1;
        }
    }
}

char* getRowValue(char** rows, size_t rowIdx, size_t columnIdx, size_t rowCount) {
    size_t i = 0;
    size_t commas = 0;
    if (rowIdx > rowCount){
        return NULL;
    }
    size_t start = 0;
    char* row = rows[rowIdx];
    while (1) {
        if ((i > 0 && row[i] == ',' && row[i - 1] != '\\') || row[i] == '\n') {
            commas++;
            if(commas - 2 == columnIdx){
                char *value = createBufferWithSize(i - start);
                strncpy(value, row + start, i - start);
                value[i - start] = '\0';
                return value;
            }
            else{
                start = i + 1;
            }
            if(row[i] == '\n'){
                break;
            }
        }
        i++;
    }
    return NULL;
}

int doesTableExist(char* table){
    int exists = 0;
    NodeList tables = loadTables();
    Node *node = getNodeFromList(&tables, table);
    if(node != NULL){
        exists = 1;
        free(node);
        free(tables.nodes);
        free(tables.tables);
    }
    return exists;
}

