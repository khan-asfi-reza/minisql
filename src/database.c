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
char* getTableSQLPathName(Node node){
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


int hasColumn(Column* columns, int numberOfCol, const char* column){
    for (int i = 0; i < numberOfCol; ++i) {
        if(caseInsensitiveCompare(columns[i].columnToken.value, column) == 0){
            return 1;
        }
    }
    return 0;
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
 * Creates a new table based on the specified SQL Node.
 * This function checks if the table already exists, and if not, it proceeds to create the table.
 * It handles the creation of necessary files and updates the table configuration.
 *
 * @param sqlNode Parsed SQL Node representing the 'CREATE TABLE' statement.
 * @return DBOp A Database Operation object containing the result of the table creation process.
 */
DBOp dbCreateTable(Node sqlNode){
    // Initialize a database operation object for this operation.
    DBOp dbOperation = createDBOp();
    // Obtain file names for the table data, SQL, and configuration.
    char* tableDataFileName = getTableDataFileName(sqlNode);
    char* tableSql = getTableSQLPathName(sqlNode);
    char* tableConfStr = getTableConfFileName();
    char *pKeyFile;
    // Check if the table already exists by verifying the presence of its data file or SQL file.
    if(fileExists(tableDataFileName) || fileExists(tableSql)){
        insertInBuffer(&dbOperation.error, "Table `%s` already exists", sqlNode.table.value);
        dbOperation.code = FAIL; // Set operation status to failure if table exists.
    }
    // Proceed if the table does not exist.
    if(dbOperation.code == SUCCESS){
        // Add the new table's SQL file path to the table configuration.
        int writeInTableConf = putInFile(tableConfStr, tableSql, 1);
        // Check for issues with writing to the table configuration file.
        if(writeInTableConf == 0){
            insertInBuffer(&dbOperation.error, "Database is corrupted, reinstall or fix config file", sqlNode.table.value);
            dbOperation.code = FAIL;
        }
    }
    // Continue if the previous steps were successful.
    if(dbOperation.code == SUCCESS){
        // Check if the table has an "id" column to treat as a primary key.
        if(hasColumn(sqlNode.columns, sqlNode.colsLen, "id") == 0){
            // Create a file for the primary key.
            pKeyFile = getTablePkName(sqlNode);
            writeInFile(pKeyFile, "0");
            free(pKeyFile); // Free the allocated memory for the primary key file name.
        }

        // Write the SQL statement to the table's SQL file and create the table data file.
        writeInFile(tableSql, sqlNode.sql);
        createFile(tableDataFileName);
        insertInBuffer(&dbOperation.successMsg, "Created table `%s`", sqlNode.table.value);
    }

    // Free the memory allocated for the file name strings.
    freeMultiple(3, tableDataFileName, tableSql, tableConfStr);

    return dbOperation;
}


/**
 * Get index in the column array of a given column
 * @param sqlNode Parsed SQL Node
 * @param column name of the column
 * @return index or -1
 */
int getColumnIndex(Node* node, char* column){
    for (int i = 0; i < node->colsLen ; i++) {
        if(caseInsensitiveCompare(node->columns[i].columnToken.value, column) == 0){
            return i;
        }
    }
    return -1;
}


/**
 * Matches column value for a given column, use case: duplicate value checking
 * @param table Name of the table file
 * @param colIdx Index of the column in the column array
 * @param str Value to compare
 * @return 0 or 1, 0 = Not match, 1 = Match
 */
int matchColumnValue(char* table, size_t colIdx, char* str){
    FILE *file;
    file = fopen(table, "r");
    char *line = NULL;
    size_t len = 0;
    // File cannot be null
    if (file == NULL) {
        printError("Database Table `%s` corrupted", table);
        return -1;
    }
    int lineCount = 0;
    // Get line gets line from a text and stores in the line variable
    while (getLine(&line, &len, file) != -1) {
        int start = 0;
        int i = 0;
        size_t commas = 0;
        while (1){
            if(line[i] == ','){
                // Increment the comma count. This count tracks the number of commas encountered while parsing the line.
                commas++;
                /* Explanation of the File Format and Parsing Logic:
                - The file contains lines formatted like "1,value1,value2,value3,value4".
                - The first number (e.g., '1' here) indicates whether the row is valid (not deleted).
                - Values are separated by commas, and the index of each value starts at 0.
                - For example, to access the value at index 2, we need to extract 'value3'.
                - When a comma is encountered, the 'start' is set to the character position following the comma.
                - The target column index (colIdx) is offset by 2 due to the initial validity indicator and zero-based indexing.
                - When the count of commas matches 'colIdx + 2', the desired value is reached.
                - 'value' is then extracted from the line starting from 'start' to the current position 'i'.
                - This extracted 'value' is compared with the target string 'str'.
                - If they match, return 1 (indicating a successful match).
                - If there is no match, the loop is exited, and the search in this line concludes.
                */
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

/**
 * Loads tables from a configuration file and creates a node list representing the abstract syntax tree (AST) for each SQL statement found.
 *
 * The function opens a configuration file that lists SQL table filenames. For each filename listed,
 * it reads the first line of the corresponding SQL file, performs lexical analysis on it,
 * creates an AST node from the tokens, and adds the node to the node list.
 * The function also handles file opening errors and database corruption issues.
 *
 * @return NodeList A list of nodes representing the AST of each SQL statement in the loaded tables.
 */
NodeList loadTables() {
    FILE *file;
    NodeList nodeList = emptyNodeList(); // Initialize an empty node list.

    // Define the path to the table configuration file.
    char* tableConfStr = createBuffer();
    insertInBuffer(&tableConfStr, "%s/.table", DATA_DIR);
    // Open the table configuration file.
    file = fopen(tableConfStr, "r");

    char *line = NULL;
    size_t len = 0;

    // Check if the file is successfully opened.
    if (file == NULL) {
        printError("Database corrupted"); // Print error if file opening fails.
        return nodeList; // Return the empty node list.
    }

    size_t size = 0; // Counter for the number of SQL files processed.

    // Read each line (table file name) from the configuration file.
    while ((getLine(&line, &len, file)) != -1) {
        size_t s_len = strlen(line);
        line[s_len-1] = '\0'; // Remove the newline character from the end of the line.

        // Open the SQL file specified in the line.
        FILE *sqlFile = fopen(line, "r");
        size_t internalLen = 0;

        if (sqlFile != NULL) {
            size++; // Increment the counter for successful file opening.
            char *sql = NULL;

            // Read the first line from the SQL file.
            getLine(&sql, &internalLen, sqlFile);

            // Perform lexical analysis on the SQL statement.
            TokenRet tokenRet = lexAnalyze(sql);

            // Create an AST node from the tokens.
            Node node = createASTNode(tokenRet);

            // Allocate memory for the new node and add it to the node list.
            Node *newNode = malloc(sizeof(node) * 1);
            *newNode = node;
            insertInNodeList(&nodeList, newNode);

        }
    }

    // Set the size of the node list.
    nodeList.size = size;
    // Clean up: free the line buffer and close the configuration file.
    free(line);
    fclose(file);
    return nodeList;
}

/**
 * Constructs a DBOp (Database Operation) object with header information based on SQL and table nodes.
 *
 * This function takes an SQL node and a table node as input. It creates a header for a database operation
 * based on these nodes. The header includes column names and related metadata. If the SQL node represents
 * a 'SELECT *' query or an 'INSERT' operation, the columns from the table node are used. Otherwise,
 * the columns are taken from the SQL node. The function also checks for the validity of the columns and
 * calculates the maximum column size for formatting purposes.
 *
 * @param sqlNode Node representing the SQL operation.
 * @param tableNode Node representing the table structure.
 * @return DBOp A Database Operation object populated with header information.
 */
DBOp createDbOpWithHeader(Node sqlNode, Node tableNode) {
    // Create an initial DBOp object.
    DBOp header = createDBOp();
    Node sNode = sqlNode;

    // If SQL node indicates all columns or is an INSERT operation, use the table node instead.
    if (sqlNode.isAllCol || isInsertKeyword(sqlNode.action.value)) {
        sNode = tableNode;
    }

    int i = 0; // Column index.
    for (; i < sNode.colsLen; ++i) { // Iterate through the columns.
        // Get the index of the column in the table node.
        int col_idx = getColumnIndex(&tableNode, sNode.columns[i].columnToken.value);

        if (col_idx == -1) {
            // Error handling for invalid column names.
            printError(
                    "Invalid column `%s`, column `%s` doesn't exist in table",
                    sNode.columns[i].columnToken.value,
                    sNode.columns[i].columnToken.value,
                    sNode.table.value
            );
            clearBuffer(&header.error);
            header.code = FAIL;
            return header; // Return header with error code if column is invalid.
        }
        else if (col_idx < COL_MAX_SIZE) {
            // Update max column space for formatting.
            header.maxColSpace = getMaxColSize(header.maxColSpace, strlen(tableNode.columns[col_idx].columnToken.value));
            // Insert column name into the result buffer.
            insertInBuffer(&header.result, "%s", tableNode.columns[col_idx].columnToken.value);
        }

        // Add a comma or newline after the column name, depending on its position.
        if (i != sNode.colsLen - 1) {
            insertInBuffer(&header.result, ",");
        } else {
            insertInBuffer(&header.result, "\n");
        }
    }

    // Set additional header properties based on the processed columns.
    header.lineCount = 1; // Indicates the number of lines in the header.
    header.colCount = i; // Total number of columns processed.
    header.action = createBufferWithSize(strlen(sNode.action.value)); // Create a buffer for the SQL action.
    insertInBuffer(&header.action, "%s", sqlNode.action.value); // Insert the SQL action into the buffer.

    return header; // Return the constructed DBOp object.
}

/**
 * Retrieves the primary key value from a primary key file.
 * This function reads the first line from the given primary key file and attempts to parse it as a numeric value.
 * It checks for validity of the data and returns the primary key. In case of any error, it prints relevant
 * error messages indicating either a conversion or read error, suggesting file corruption.
 *
 * @param pkFile A file pointer to an open primary key file.
 * @return size_t The primary key value on success, or (size_t)-1 on failure.
 */
size_t getPkFromPkFile(FILE *pkFile){
    if (pkFile != NULL) {
        // Variable to store the primary key.
        size_t _id;

        // Buffer to read the line from the file.
        char buffer[64];

        // Read the first line from the file.
        if (fgets(buffer, sizeof(buffer), pkFile) != NULL) {
            // Pointer to track the end of the numeric parsing.
            char *endPointer;

            // Convert the buffer to a long integer.
            long int temp_id = strtol(buffer, &endPointer, 10);

            // Check if the conversion was successful.
            if (*endPointer == '\0' || *endPointer == '\n') {
                _id = (size_t)temp_id;
                return _id; // Return the parsed primary key.
            } else {
                // Error handling for invalid data in the primary key file.
                printError("Invalid pk file, pk file is corrupted\n");
                fprintf(stderr, "Conversion error occurred\n");
            }
        } else {
            // Error handling for read failure.
            printError("Invalid pk file, pk file is corrupted\n");
            fprintf(stderr, "Read error occurred\n");
        }
    }
    // Return an error code if pkFile is NULL or in case of any error.
    return (size_t)-1;
}

/**
 * Deletes specified lines from a file.
 * This function opens a file, reads its contents, and creates a new buffer where it copies
 * all lines except those specified to be deleted. It then writes this new buffer back to the file,
 * effectively removing the specified lines. The lines to be deleted are identified by their line numbers.
 *
 * @param filename The name of the file from which lines are to be deleted.
 * @param lines An array of line numbers to be deleted.
 * @param size The number of elements in the 'lines' array.
 * @return int 0 on successful deletion, -1 on failure.
 */
int deleteLine(const char *filename, const size_t *lines, size_t size) {
    FILE *file;
    char *buffer, *newBuffer;
    long length;
    size_t line_count = 1; // Counter for the current line number.
    size_t current_line = 0; // Index for the 'lines' array.
    char *cursor, *next_line;

    // Open the file for reading.
    file = fopen(filename, "r");
    if (file == NULL) {
        printError("Unable to open file for deletion");
        return -1;
    }

    // Determine the length of the file.
    // Move the file pointer to the end of the file.
    fseek(file, 0, SEEK_END);
    // Determine the length of the file.
    // 'ftell' returns the current file position of the file pointer (which is now at the end of the file),
    // effectively giving us the total length of the file in bytes.
    length = ftell(file);

    // Rewind the file pointer back to the start of the file.
    // This is necessary to read the file from the beginning in the subsequent operations.
    rewind(file);

    // Allocate memory for reading the entire file.
    buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        printError("Error allocating memory");
        fclose(file);
        return -1;
    }

    // Read the file into the buffer.
    fread(buffer, length, 1, file);
    fclose(file);
    buffer[length] = '\0'; // Null-terminate the buffer.

    // Allocate memory for the new buffer that will hold the modified file content.
    newBuffer = (char *)malloc(length + 1);
    if (newBuffer == NULL) {
        printError("Error allocating memory for new buffer");
        free(buffer);
        return -1;
    }

    cursor = buffer; // Set cursor to the beginning of the buffer.
    next_line = NULL;
    size_t newBufferIdx = 0; // Index for writing to the new buffer.

    // Loop through the buffer, skipping lines that need to be deleted.
    while (line_count <= lines[size - 1] && *cursor != '\0') {
        // Check if the current line is in the 'lines' array to be deleted.
        if (current_line < size && line_count == lines[current_line]) {
            next_line = strchr(cursor, '\n'); // Find the end of the current line.
            cursor = (next_line != NULL) ? next_line + 1 : cursor + strlen(cursor); // Move cursor to the next line.
            current_line++;
        } else {
            // Copy the line to the new buffer.
            next_line = strchr(cursor, '\n');
            size_t line_length = (next_line != NULL) ? (size_t)(next_line - cursor) + 1 : strlen(cursor);
            memcpy(newBuffer + newBufferIdx, cursor, line_length);
            newBufferIdx += line_length;
            cursor += line_length;
        }
        line_count++;
    }

    // Copy any remaining content to the new buffer.
    if (*cursor != '\0') {
        strcpy(newBuffer + newBufferIdx, cursor);
    }
    int retCode = 0;
    // Open the file for writing and write the new buffer to it.
    file = fopen(filename, "w");
    if (file == NULL) {
        printError("Error opening file");
        retCode = -1;
    }
    else{
        fwrite(newBuffer, strlen(newBuffer), 1, file);
        fclose(file);
    }
    // Clean up: free the allocated buffers.
    freeMultiple(2, newBuffer, buffer);
    return retCode;
}

/**
 * Determines whether a value should be included in a result set based on a filter condition.
 * This function compares a given value with a specified filter condition and determines if the value
 * meets the criteria defined by the filter. The filter condition includes a comparison operator and a value.
 * It supports both numerical comparisons and direct string comparisons.
 *
 * @param filter The filter condition, which includes the comparison operator and value.
 * @param value The value to be compared against the filter condition.
 * @return int 1 if the value meets the filter condition (should be included), 0 otherwise.
 */
int filterValue(Column filter, const char* value){
    int shouldInsertInRow = 1; // Default to true (the value meets the condition).
    // Remove single quotes from the filter value for comparison.
    removeSingleQuotes(filter.valueToken.value);
    // Check if both the filter value and the given value are numbers for numerical comparison.
    if(filter.valueToken.type == TOKEN_NUMBER && isNumber(value)){
        size_t val = strToLongInt(value); // Convert the given value to a long integer.
        size_t cmpVal = strToLongInt(filter.valueToken.value); // Convert the filter value to a long integer.
        // Compare the values based on the filter's comparison operator.
        if(strcmp(filter.symbol.value, "<") == 0){
            shouldInsertInRow = (val < cmpVal);
        }
        else if(strcmp(filter.symbol.value, ">") == 0){
            shouldInsertInRow = (val > cmpVal);
        }
        else if(strcmp(filter.symbol.value, ">=") == 0){
            shouldInsertInRow = (val >= cmpVal);
        }
        else if(strcmp(filter.symbol.value, "<=") == 0){
            shouldInsertInRow = (val <= cmpVal);
        }
        else if(strcmp(filter.symbol.value, "!=") == 0){
            shouldInsertInRow = (val != cmpVal);
        }
        else if(strcmp(filter.symbol.value, "=") == 0){
            shouldInsertInRow = (val == cmpVal);
        }
        else{
            // If the operator is not recognized, default to false.
            shouldInsertInRow = 0;
        }
    }
        // If not numerical comparison, perform a direct string comparison.
    else if(strcmp(filter.valueToken.value, value) != 0){
        shouldInsertInRow = 0; // The strings do not match.
    }
    return shouldInsertInRow;
}

/**
 * Performs an update operation on a database table based on the specified SQL node.
 * The function reads through the table file, applies the update conditions defined in the SQL node,
 * and modifies the rows that meet these conditions. It handles unique value constraints and updates the table accordingly.
 *
 * @param sqlNode The SQL node containing the update statement and conditions.
 * @param tableNode The node representing the table structure and metadata.
 * @return DBOp A Database Operation object containing the result of the update operation, including any errors encountered.
 */
DBOp dbUpdate(Node sqlNode, Node tableNode){
    Node sNode = sqlNode; // Use sqlNode directly, unless it specifies all columns.
    if(sqlNode.isAllCol){
        sNode = tableNode; // Use tableNode if all columns are specified in the SQL node.
    }

    // Create a database operation with headers based on SQL and table nodes.
    DBOp dbOp = createDbOpWithHeader(sqlNode, tableNode);

    size_t upCount = 0; // Counter for the number of rows updated.
    char *tableName = getTableDataFileName(sqlNode); // Get the file name for the table data.
    char* tBuffer = createBuffer(); // Buffer to store table data for writing back to file.
    FILE *tableFile = fopen(tableName, "r+"); // Open the table file for reading and writing.
    char **rows = malloc(sizeof(char*) * 1); // Array to store updated rows.
    size_t rowCount = 0; // Counter for the number of rows stored in 'rows'.

    if(tableFile != NULL){
        char *line = NULL; // Buffer to store each line read from the file.
        size_t len = 0; // Length of the line.
        int lineCount = 0; // Count of lines processed.

        // Read each line from the table file.
        while (getLine(&line, &len, tableFile) != -1){
            lineCount++;
            int shouldInsertInRow = 1; // Flag to determine if the current row should be updated.

            // Process each filter in the SQL node.
            for (int fil = 0; fil < sNode.filtersLen; ++fil) {
                Column filter = sNode.filters[fil]; // Get the current filter.
                int colIdx = getColumnIndex(&tableNode, filter.columnToken.value); // Find the column index for the filter.

                // Variables for parsing the line and locating the relevant value.
                size_t start = 0;
                size_t i = 0;
                size_t commas = 0;

                // Parse the line to find the value corresponding to the filter column.
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
                    break; // Break if the row does not meet the filter conditions.
                }
            }

            // Update the row if it meets all filter conditions.
            if(shouldInsertInRow == 1){
                upCount++;
                // Process each column in the SQL node for updating.
                for (int col = 0; col < sNode.colsLen; ++col) {
                    Column column = sNode.columns[col];
                    int colIdx = getColumnIndex(&tableNode, column.columnToken.value);
                    size_t iStart = 0;
                    size_t j = 0;
                    size_t commas = 0;

                    // Parse the line to find and update the relevant value.
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
                                    // Handle unique constraint violation.
                                    dbOp.code = FAIL;
                                    insertInBuffer(&dbOp.error, "Duplicate value `%s` for column `%s` violates unique constraint", column.valueToken.value, column.columnToken.value);
                                    freeMultiple(3, tableName, tBuffer, line);
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

                // Reallocate memory for the rows array to accommodate the new row.
                char **tempRow = realloc(rows, sizeof(char*) * rowCount);
                if(tempRow != NULL){
                    rows = tempRow;
                }
                else{
                    // Handle memory allocation failure.
                    dbOp.code = FAIL;
                    insertInBuffer(&dbOp.error, "MEM Failed");
                    freeMultiple(3, tableName, tBuffer, line);
                    return dbOp;
                }
            }
            insertInBuffer(&tBuffer, "%s", line); // Append the line to the buffer for writing back.
        }
        fclose(tableFile); // Close the table file.
        dbOp.lineCount += lineCount; // Update the line count in the database operation.
    }

    // Set the rows and row count in the database operation.
    dbOp.rows = rows;
    dbOp.rowCount = rowCount;

    // Write the updated data back to the table file.
    writeInFile(tableName, tBuffer);

    // Clean up and set the success message.
    clearBuffer(&tBuffer);
    insertInBuffer(&dbOp.successMsg, "Updated `%zd` rows in table %s", upCount, sNode.table.value);
    // Free allocated resources.
    free(tableName);
    return dbOp;
}

/**
 * Performs a select operation on a database table based on the specified SQL node.
 * This function reads through the table file and applies the select conditions defined in the SQL node.
 * It collects and returns the rows that meet these conditions. Supports logical operations in filtering.
 *
 * @param sqlNode The SQL node containing the select statement and conditions.
 * @param tableNode The node representing the table structure and metadata.
 * @return DBOp A Database Operation object containing the result of the select operation.
 */
DBOp dbSelect(Node sqlNode, Node tableNode){
    Node sNode = sqlNode; // Use sqlNode directly, unless it specifies all columns.
    if(sqlNode.isAllCol){
        sNode = tableNode; // Use tableNode if all columns are specified in the SQL node.
    }

    // Create a database operation with headers based on SQL and table nodes.
    DBOp dbOp = createDbOpWithHeader(sqlNode, tableNode);

    // Get the file name for the table data and open the table file.
    char *tableName = getTableDataFileName(sqlNode);
    FILE *tableFile = fopen(tableName, "r");

    // Allocate memory for storing rows that match the select criteria.
    char **rows = malloc(sizeof(char*) * 1);
    size_t rowCount = 0;

    if(tableFile != NULL){
        char *line = NULL; // Buffer to store each line read from the file.
        size_t len = 0; // Length of the line.
        int lineCount = 0; // Count of lines processed.

        // Read each line from the table file.
        while (getLine(&line, &len, tableFile) != -1){
            lineCount++;
            char* rowBuffer = createBuffer(); // Buffer to store a row that matches the select criteria.
            int shouldInsertInRow = 1; // Flag to determine if the current row matches the criteria.

            // Process each filter in the SQL node.
            for (size_t fil = 0; fil < sqlNode.filtersLen; ++fil) {
                // Variables for parsing the line and locating the relevant value.
                size_t start = 0;
                size_t i = 0;
                size_t commas = 0;
                int col_idx = getColumnIndex(&tableNode, sqlNode.filters[fil].columnToken.value);

                // Parse the line to find the value corresponding to the filter column.
                while (1){
                    if((i>0 && line[i] == ',' && line[i-1] != '\\') || line[i] == '\n'){
                        commas++;
                        if(commas - 2 == col_idx){
                            // Extract the value and apply the filter condition.
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
                            break; // Break if the end of the line is reached.
                        }
                    }
                    i++;
                }
                // Process logical operations (AND, OR) for combining multiple filters.
                if(sqlNode.filters[fil].nextLogicalOp.value != NULL && caseInsensitiveCompare(sqlNode.filters[fil].nextLogicalOp.value, "AND") == 0){
                    if(shouldInsertInRow == 0){
                        break; // Break if the row does not meet the filter conditions (AND logic).
                    }
                }else{
                    if(shouldInsertInRow == 1){
                        break; // Break if the row meets at least one filter condition (OR logic).
                    }
                }
            }
            // Add the row to the result if it matches all filter conditions.
            if(shouldInsertInRow == 1){
                // Extract and store the selected columns of the row.
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
                                // Insert the value into the row buffer and update column space size.
                                insertInBuffer(&rowBuffer, "%s", value);
                                dbOp.maxColSpace = getMaxColSize(dbOp.maxColSpace, strlen(value));
                                clearBuffer(&value);
                            }
                            else{
                                start = i + 1;
                            }
                            clearBuffer(&value);
                            if(line[i] == '\n'){
                                break; // Break if the end of the line is reached.
                            }
                        }
                        i++;
                    }
                    if(col != sNode.colsLen - 1){
                        insertInBuffer(&rowBuffer, ","); // Add comma between columns.
                    }
                }
                insertInBuffer(&rowBuffer, "\n"); // Add newline after each row.
                insertInBuffer(&dbOp.result, "%s", rowBuffer); // Add the row to the result buffer.

                // Store the original line in the rows array.
                rows[rowCount] = createBuffer();
                insertInBuffer(&rows[rowCount], line);
                rowCount++;

                // Reallocate memory for the rows array to accommodate the new row.
                char **tempRow = realloc(rows, sizeof(char*) * rowCount);
                if(tempRow != NULL){
                    rows = tempRow;
                }
                else{
                    // Handle memory allocation failure.
                    dbOp.code = FAIL;
                    insertInBuffer(&dbOp.error, "MEM Failed");
                    free(tableName);
                    free(line);
                    return dbOp;
                }
                clearBuffer(&rowBuffer);
            }
        }
        fclose(tableFile); // Close the table file.
        dbOp.lineCount += lineCount; // Update the line count in the database operation.
    }
    // Set the rows and row count in the database operation.
    dbOp.rows = rows;
    dbOp.rowCount = rowCount;
    return dbOp;
}

/**
 * Performs a delete operation on a database table based on the specified SQL node.
 * This function reads through the table file and applies the delete conditions defined in the SQL node.
 * It collects the line numbers of rows that meet these conditions and deletes them from the file.
 *
 * @param sqlNode The SQL node containing the delete statement and conditions.
 * @param tableNode The node representing the table structure and metadata.
 * @return DBOp A Database Operation object containing the result of the delete operation.
 */
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

/**
 * Performs an insert operation into a database table based on the specified SQL node.
 * This function appends a new row to the table file with values specified in the SQL node.
 * It also handles unique constraints and auto-incremented primary keys.
 *
 * @param sqlNode The SQL node containing the insert statement and values.
 * @param tableNode The node representing the table structure and metadata.
 * @return DBOp A Database Operation object containing the result of the insert operation.
 */
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


DBOp execSQL(char* input, NodeList *tables){
    TokenRet tokenRet = lexAnalyze(input);
    Node node = createASTNode(tokenRet);
    if(node.isInvalid == 0){
        Node *tableNode = getNodeFromList(tables, node.table.value);
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
                return dbOp;
            }
            else{
                printError("Table `%s` doesn't exist", node.table.value);
            }
        }

    }
    destroyNode(&node);
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

int doesTableExist(NodeList *tables, char* table){
    int exists = 0;;
    Node *node = getNodeFromList(tables, table);
    if(node != NULL){
        exists = 1;
    }
    return exists;
}

