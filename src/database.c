#include "lexer.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "utils.h"
#include "const.h"
#include "filesystem.h"
#include "database.h"
#include <time.h>

char* getTableName(Node node){
    const char* tableStrList[] = {DATA_DIR, "/table_", node.table.value};
    char* tableFullName = concatStrings(tableStrList, 3);
    return tableFullName;
}

char* getTableSQLName(Node node){
    const char* tableSqlStrList[] = {DATA_DIR, "/table_", node.table.value, "_sql"};
    char* tableSql = concatStrings(tableSqlStrList, 4);
    return tableSql;
}

char* getTablePkName(Node node){
    const char* tablePkList[] = {DATA_DIR, "/table_", node.table.value, "_pk"};
    char* tablePk = concatStrings(tablePkList, 4);
    return tablePk;
}


size_t getMaxColSize(size_t a, size_t b){
    size_t c = max(a,b);
    if((c & 1) == 0){
        c = c+1;
    }
    if(c < 13){
        return 13;
    }
    if(c > MAX_COL_SIZE){
        return MAX_COL_SIZE;
    }
    return c;
}

DBOp createDBOp(){
    DBOp dbOperation;
    dbOperation.code = SUCCESS;
    dbOperation.successMsg = createBuffer();
    dbOperation.error = createBuffer();
    dbOperation.result = createBuffer();
    dbOperation.maxColSpace = 5;
    dbOperation.lineCount = 0;
    dbOperation.colCount = 0;
    return dbOperation;
}

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
}

DBOp dbCreateTable(Node sqlNode){

    DBOp dbOperation = createDBOp();

    FILE *tableFile = NULL, *tableSqlFile = NULL;
    char* tableFullName = getTableName(sqlNode);
    char* tableSql = getTableSQLName(sqlNode);

    const char* tableConf[] = {DATA_DIR, "/", ".table"};
    char* tableConfStr = concatStrings(tableConf, 3);
    FILE *tableConfig = NULL;
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



char* defaultValue(Token token){
    if(caseInsensitiveCompare(token.value, "NOW") == 0){
        time_t now;
        struct tm *utc;
        char *dateTimeStr;
        dateTimeStr = (char *)malloc(100 * sizeof(char));
        if (dateTimeStr == NULL) {
            printError("Memory allocation failed while creating timestamp.\n");
            return "";
        }
        time(&now);
        int offsetHours = 1;
        now += offsetHours * 3600;
        utc = gmtime(&now);
        if (strftime(dateTimeStr, 100, "%Y-%m-%d %H:%M:%S GMT+1", utc) == 0) {
            printError("Failed to format date-time string.\n");
            free(dateTimeStr);
            return "";
        }
        return dateTimeStr;
    }
    return "1";
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
    while (getline(&line, &len, file) != -1) {
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

    while ((getline(&line, &len, file)) != -1) {
        size_t s_len = strlen(line);
        line[s_len-1] = '\0';
        FILE *sqlFile = fopen(line, "r");
        size_t internalLen = 0;
        if(sqlFile != NULL){
            char *sql = NULL;
            getline(&sql, &internalLen, sqlFile);
            TokenRet tokenRet = lexAnalyze(sql);
            Node node = createASTNode(tokenRet);
            Node *newNode = malloc(sizeof(node) * 1);
            *newNode = node;
            insertInNodeList(&nodeList, newNode);
            free(sql);
        }

    }

    free(line);
    fclose(file);
    return nodeList;
}

struct {
    char* backup;
    long fileSize;
} typedef BackupTableFile;


BackupTableFile getTableBackup(char* tableName){
    BackupTableFile btf = {};
    FILE *file = fopen(tableName, "r+");
    if (!file) {
        perror("Failed to open file");
        return btf;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    char *backup = malloc(file_size);

    if (!backup) {
        perror("Failed to allocate memory");
        fclose(file);
        return btf;
    }
    fseek(file, 0, SEEK_SET);
    fread(backup, 1, file_size, file);
    btf.backup = backup;
    btf.fileSize = file_size;
    fclose(file);
    return btf;
}

void restoreTable(char* tableName, BackupTableFile tableBackup) {
    FILE *file = fopen(tableName, "w");
    if(file != NULL){
        fseek(file, 0, SEEK_SET);
        fwrite(tableBackup.backup, 1, tableBackup.fileSize, file);
    }
    fclose(file);
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
    char *tableName = getTableName(sqlNode);
    char* tBuffer = createBuffer();
    FILE *tableFile = fopen(tableName, "r+");
    if(tableFile != NULL){
        char *line = NULL;
        size_t len = 0;
        int lineCount = 0;
        while (getline(&line, &len, tableFile) != -1){
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
            }
            insertInBuffer(&tBuffer, "%s", line);
        }
        fclose(tableFile);
        dbOp.lineCount += lineCount;
    }
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
    char *tableName = getTableName(sqlNode);
    FILE *tableFile = fopen(tableName, "r");
    if(tableFile != NULL){
        char *line = NULL;
        size_t len = 0;
        int lineCount = 0;
        while (getline(&line, &len, tableFile) != -1){
            lineCount++;
            char* rowBuffer = createBuffer();
            int shouldInsertInRow = 1;
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
                        int toFindColumn = (int)commas - 2;
                        if(toFindColumn >= 0){
                            Column fCol = tableNode.columns[toFindColumn];
                            int filterColExist = getFilterIndex(&sqlNode, fCol.columnToken.value);
                            if(filterColExist != -1){
                                Column filter = sqlNode.filters[filterColExist];
                                if(filter.valueToken.value != NULL){
                                    removeSingleQuotes(filter.valueToken.value);
                                    shouldInsertInRow = filterValue(filter, value);
                                }
                                else{
                                    dbOp.code = FAIL;
                                    clearBuffer(&dbOp.result);
                                    insertInBuffer(&dbOp.error, "Invalid filter statement");
                                    return dbOp;
                                }
                            }
                        }

                        if(shouldInsertInRow == 1 && commas == col_idx + 2){
                            insertInBuffer(&rowBuffer, "%s", value);
                            dbOp.maxColSpace = getMaxColSize(dbOp.maxColSpace, strlen(value));
                            clearBuffer(&value);
                            break;
                        }
                        else{
                            start = i + 1;
                        }
                        clearBuffer(&value);
                        if(shouldInsertInRow == 0){
                            break;
                        }
                    }
                    i++;
                }
                if(shouldInsertInRow == 1 && col != sNode.colsLen - 1){
                    insertInBuffer(&rowBuffer, ",");
                }
                if(shouldInsertInRow == 0){
                    break;
                }
            }
            if(shouldInsertInRow == 1){
                insertInBuffer(&rowBuffer, "\n");
                insertInBuffer(&dbOp.result, "%s", rowBuffer);
            }
            clearBuffer(&rowBuffer);
        }
        fclose(tableFile);
        dbOp.lineCount += lineCount;
    }
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
    char *tableName = getTableName(sqlNode);
    FILE *tableFile = fopen(tableName, "r");
    if(tableFile != NULL){
        char *line = NULL;
        size_t len = 0;
        int lineCount = 0;
        while (getline(&line, &len, tableFile) != -1){
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
    char* tableName = getTableName(sqlNode);
    if(dbOp.code != SUCCESS){
        return dbOp;
    }
    size_t _id = -1;
    FILE *pkFile;
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

    free(tableName);
    return dbOp;
}


void printDbOp(DBOp *dbOp){
    char* result = dbOp->result;
    size_t start = 0;
    size_t pIdx = 0;
    size_t inLineIdx;
    int isInLine = 0;
    size_t mSpace = dbOp->maxColSpace;
    int isEvenTOff;
    for (int i = 0; i < dbOp->colCount * mSpace + dbOp->colCount; ++i) {
        printf("_");
    }
    printf("\n");
    size_t prevPIdx = pIdx;
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

            if(offset == 0){
                isInLine = 1;
                inLineIdx = prevPIdx;
            }

            if(isInLine == 0){
                for (int j = 0; j < offset/2; ++j) {
                    printf(" ");
                    pIdx++;
                }
                if(isEvenTOff == 1){
                    printf(" ");
                    pIdx++;
                }
                for (size_t l = start; l < i; ++l) {
                    printf("%c", result[l]);
                    pIdx++;
                }
                for (int j = 0; j < offset/2; ++j) {
                    printf(" ");
                    pIdx++;
                }
            }
            else{
                size_t lIdx = inLineIdx;
                printf(" ");
                for (size_t f = start; f < i; ++f) {
                    if(lIdx >= inLineIdx && lIdx < inLineIdx + mSpace - 2){
                        printf("%c", result[f]);
                        lIdx++;
                    }
                    else{
                        size_t xCtr = 0;
                        printf(" |\n");
                        for (int j = 0; j < inLineIdx ; ++j) {
                            if (xCtr == mSpace){
                                printf("|");
                                xCtr = -1;
                            }
                            else{
                                printf(" ");
                            }
                            xCtr++;

                        }
                        lIdx = inLineIdx;
                        f--;
                        printf(" ");
                    }
                }
                while(lIdx >= inLineIdx && lIdx < inLineIdx + mSpace - 1){
                    printf(" ");
                    lIdx++;
                }
                isInLine = 0;
            }
            printf("|");
            pIdx++;
            prevPIdx = pIdx;
            if(result[i] == '\n'){
                printf("\n");
                pIdx = 0;
                isInLine = 0;
                for (int x = 0; x < dbOp->colCount * mSpace + dbOp->colCount; ++x) {
                    printf("-");
                }
                printf("\n");
            }
            start = i + 1;
        }
    }
}