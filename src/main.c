#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "const.h"
#include "lexer.h"
#include "io.h"
#include "filesystem.h"
#include "database.h"
#include "stdbool.h"

#define MAX_LENGTH 32

void printIntroText(){
    // Color green start
    printf("\033[0;32m");
    printf("MiniSql - v0.1\n");
    printf("=====================================================\n");
    printf("m    m   iii   nnnn    iii   ssss    q q q       ll\n");
    printf("mm  mm    i    n   n    i    ss      q   q    q  ll   \n");
    printf("m mm m    i    n   n    i     sss    qqqqq   q   ll   \n");
    printf("m    m    i    n   n    i        ss      q q     ll   \n");
    printf("m    m   iii   n   n   iii    ssss       qq      lllll\n");
    printf("=====================================================\n\n");
    // Color green end
    printf("\033[0m");
}

// User struct for username and password also confirm the password
struct {
    char username[MAX_LENGTH];
    char password[MAX_LENGTH];
    char confirmPassword[MAX_LENGTH];
} typedef User;



/** It takes an integer argument to determine if password confirmation is needed.
 *  Gets user authentication info from stdin. Verifies if the username is valid
 *  If confirmPassword flag is up then checks for password and Confirm Password inputs
 *  Recursively asks for input.
 *  @param confirmPassword If confirming password is necessary
 *  @returns validated User information
 * */
 User getUserInfo(int confirmPassword){
    // Create a User struct instance to store the user's details.
    fflush(stdin);
    User user;

    // Declare a variable to track the validity of the input.
    int isValidInput;

    // Loop indefinitely until valid input is received and processed.
    while (1) {
        // Assume input is valid at the beginning of each loop iteration.
        isValidInput = 1;
        // Prompt the user for a username and read the input.
        printf("Enter username (max 32 characters): ");
        fgets(user.username, sizeof(user.username), stdin);
        // Check if the newline character is present in the input buffer.
        if (!strchr(user.username, '\n')) {
            // Clear the input buffer if the newline character is missing.
            clearInputBuffer();
            isValidInput = false;
        }
        // Remove the newline character from the username.
        user.username[strcspn(user.username, "\n")] = 0;
        // Check if username length is valid and if the input is valid.
        if (strlen(user.username) > MAX_LENGTH || !isValidInput) {
            printf("Username cannot exceed 32 characters.\n");
            continue;
        }
        // Prompt for password and read the input.
        printf("Enter password (max 32 characters): ");
        fgets(user.password, sizeof(user.password), stdin);
        // Remove the newline character from the password.
        user.password[strcspn(user.password, "\n")] = 0;
        // Check if password length is valid.
        if (strlen(user.password) > MAX_LENGTH) {
            printf("Password cannot exceed 32 characters.\n");
            continue;
        }
        // Check if password confirmation is needed.
        if(confirmPassword == 1){
            // Prompt for password confirmation and read the input.
            printf("Confirm password: ");
            fgets(user.confirmPassword, sizeof(user.confirmPassword), stdin);
            // Remove the newline character from the confirmed password.
            user.confirmPassword[strcspn(user.confirmPassword, "\n")] = 0;
            // Check if the password and confirmed password match.
            if (strcmp(user.password, user.confirmPassword) != 0) {
                printf("Passwords do not match. Try again.\n");
                continue;
            }
        }
        // Break out of the loop if all inputs are valid.
        break;
    }
    // Return the user structure with the collected information.
    return user;
}

/** Creates a user and stores in user table
 *  @param NodeList tables List of stored tables
 *  @returns 1 or 0
 * */
int createUser(TableList *tables){
    printf("Create your account\n");
    // Get user info from stdin and avoid confirming password
    User user = getUserInfo(1);
    char *buffer = createBuffer();
    // SQL To create user record in the table
    insertInBuffer(&buffer, "INSERT INTO user (username, password) VALUES ('%s', '%s');", user.username, user.password);
    // Execute sql
    DBOp dbOp = execSQL(buffer, tables);
    // Clear sql input
    clearBuffer(&buffer);
    // Print the row
    printDbOp(&dbOp);
    return 1;
}

/** Creates a data directory and table config.
 *  Data directory serves the purpose of storing actual data.
 *  Every sql, row, column and index data will be stored in the data directory.
 *  Table config stores the path of the table sql construction file. To keep
 *  track of the tables in the database, the .table config is created.
 *  In .table file sql path is stored in the following order
 *  ```
 *  data/table_user_sql
 *  data/table_table_a_sql
 *  ```
 * */
void initDataDirectory(){
    // Checks if data directory exists or not
    if (directoryExists(DATA_DIR) != 1) {
        // If not create the directory
        if (createDirectory(DATA_DIR)) {
            printf("Setting up 'minisql' database.\n");
        } else {
            printf("Failed to setup database, error: Failed to create data directory .\n");
        }
    }
    // Get the name of the config file
    char *conf = getTableConfFileName();
    // Create the table config file generally .table file
    if(fileExists(conf) == 0){
        createFile(conf);
    }
    // Clear the string from heap
    clearBuffer(&conf);
}

/** Authenticates a user to give access to the database
 *  @param user User property taken from stdin
 *  @param NodeList tables List of stored tables
 *  @returns 1 or 0, 1 = User is authenticated and 0 = Not authenticated, -1 = User doesn't exist
 * */
int initialize(TableList *tables){
    // Checks if user table exists or not.
    // User table is required to authenticate
    int exists = doesTableExist(tables, "user");
    if(exists == 0){
        // User Table must be created through sql create table command
        execSQL("CREATE TABLE user (id integer primary key, username varchar unique, password varchar, created datetime default now)", tables);
        return 0;
    }
    return 1;
}


/** Authenticates a user to give access to the database
 *  @param user User property taken from stdin
 *  @param NodeList tables List of stored tables
 *  @returns 1 or 0, 1 = User is authenticated and 0 = Not authenticated, -1 = User doesn't exist
 * */
int authenticate(User user, TableList *tables){
    // Create a buffer to create the sql statement
    char *buffer = createBuffer();
    // Generate sql with buffer
    insertInBuffer(&buffer, "SELECT username, password FROM user where username = '%s';", user.username);
    // Execute the sql
    DBOp dbOp = execSQL(buffer, tables);
    // Clear buffer as no longer needed
    clearBuffer(&buffer);
    // If fetched row is not 1, which means the row or record doesn't exist
    if(dbOp.rowCount != 1){
        // - 1 = User doesn't exist
        return -1;
    }
    else{
        // Check for password check if password matches
        char* pass = getRowValue(dbOp.rows, 0, 2, dbOp.rowCount);
        if(strcmp(pass, user.password) == 0){
            // Password matched
            return 1;
        }
    }
    // 0 = Password mismatch
    return 0;
}


/** User login system, asks for user login until password and username matches
 *  @param tables tables List of stored tables
 * */
void login(TableList *tables){
    while (1) {
        printf("Login to your account\n");
        User user = getUserInfo(0);
        int auth = authenticate(user, tables);
        if (auth == 1) {
            printSuccess("Logged in successfully");
            break;
        } else if (auth == -1) {
            printError("User doesn't exist");
        } else {
            printError("User password doesn't match");
        }
    }
}


int main() {
    // Welcome text
    printIntroText();
    // Initial directory setup / Database setup
    initDataDirectory();
    // Loads Table List
    TableList tables = loadTables();
    // Setup user table, checks if user table is required or not
    // Creates user table
    int setup = initialize(&tables);
    if (setup == 0) {
        // If newly setup creates a user
        createUser(&tables);
    }
    // Authentication system
    login(&tables);

    while (1) {
        // Takes sql input
        printf("\n$>> ");
        char *input = handleInput();
        if (input != NULL) {
            if (caseInsensitiveCompare(input, "quit;") == 0) {
                exit(0);
            } else if (caseInsensitiveCompare(input, "create user;") == 0) {
                createUser(&tables);
            } else if (caseInsensitiveCompare(input, "list tables;") == 0) {
                printTables(tables);
                tables = loadTables();
            } else {
                DBOp dbOp = execSQL(input, &tables);
                if (dbOp.code == SUCCESS) {
                    // If successfully executed print out the success message
                    printSuccess("%s", dbOp.successMsg);
                    // Show rows that were generated if only select and insert action was performed
                    if(isSelectKeyword(dbOp.action) || isInsertKeyword(dbOp.action)){
                        printDbOp(&dbOp);
                    }
                } else {
                    printError("%s", dbOp.error);
                }
                clearDBOp(&dbOp);
            }
            clearBuffer(&input);
            fflush(stdin);
            printf(" ");
        }

    }
}

