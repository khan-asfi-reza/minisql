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
    printf("\033[0;32m");
    printf("MiniSql - v0.1\n");
    printf("=====================================================\n");
    printf("m    m   iii   nnnn    iii   ssss    q q q       ll\n");
    printf("mm  mm    i    n   n    i    ss      q   q    q  ll   \n");
    printf("m mm m    i    n   n    i     sss    qqqqq   q   ll   \n");
    printf("m    m    i    n   n    i        ss      q q     ll   \n");
    printf("m    m   iii   n   n   iii    ssss       qq      lllll\n");
    printf("=====================================================\n\n");
    printf("\033[0m");
}

struct {
    char username[MAX_LENGTH];
    char password[MAX_LENGTH];
    char confirmPassword[MAX_LENGTH];
} typedef User;



void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}


User getUserInfo(int confirmPassword){
    fflush(stdin);
    User user;
    bool isValidInput;
    while (1) {
        isValidInput = true;

        printf("Enter username (max 32 characters): ");
        fgets(user.username, sizeof(user.username), stdin);
        if (!strchr(user.username, '\n')) {
            clearInputBuffer();
            isValidInput = false;
        }
        user.username[strcspn(user.username, "\n")] = 0;

        if (strlen(user.username) > MAX_LENGTH || !isValidInput) {
            printf("Username cannot exceed 32 characters.\n");
            continue;
        }

        printf("Enter password (max 32 characters): ");
        fgets(user.password, sizeof(user.password), stdin);
        user.password[strcspn(user.password, "\n")] = 0;

        if (strlen(user.password) > MAX_LENGTH) {
            printf("Password cannot exceed 32 characters.\n");
            continue;
        }

        if(confirmPassword == 1){

            printf("Confirm password: ");
            fgets(user.confirmPassword, sizeof(user.confirmPassword), stdin);
            user.confirmPassword[strcspn(user.confirmPassword, "\n")] = 0;

            if (strcmp(user.password, user.confirmPassword) != 0) {
                printf("Passwords do not match. Try again.\n");
                continue;
            }
        }

        break;
    }
    return user;

}

int createUser(Node *refNode){
    printf("Create your account\n");
    User user = getUserInfo(1);
    char *buffer = createBuffer();
    insertInBuffer(&buffer, "INSERT INTO user (username, password) VALUES ('%s', '%s');", user.username, user.password);
    TokenRet tokenRet= lexAnalyze(
            buffer
    );
    Node node__ = createASTNode(tokenRet);
    DBOp dbOp = dbInsert(node__, *refNode);
    printDbOp(&dbOp);
    return 1;
}


int initialize(){
    if (directory_exists(DATA_DIR) != 1) {
        if (create_directory(DATA_DIR)) {
            printf("Setting up 'minisql database.\n");
        } else {
            printf("Failed to setup database, error: Failed to create data directory .\n");
        }
    }
    const char* tableSqlList[] = {DATA_DIR, "/table_user_sql"};
    const char* tableNameList[] = {DATA_DIR, "/table_user"};

    char* tableSql = concatStrings(tableSqlList, 2);
    char* tableName = concatStrings(tableNameList, 2);

    if(fileExists(tableSql) == 0 || fileExists(tableName) == 0){
        TokenRet tokenRet= lexAnalyze(
                "CREATE TABLE user (id integer primary key, username varchar unique, password varchar, created datetime default now)"
        );
        Node node = createASTNode(tokenRet);
        dbCreateTable(node);
        return 0;
    }
    return 1;

}


int authenticate(User user, Node *refNode){
    char *buffer = createBuffer();
    insertInBuffer(&buffer, "SELECT username, password FROM user where username = '%s';", user.username);
    TokenRet tokenRet= lexAnalyze(
            buffer
    );
    Node node = createASTNode(tokenRet);
    DBOp dbOp = dbSelect(node, *refNode);
    if(dbOp.rowCount != 1){
        clearBuffer(&buffer);
        return -1;
    }
    else{
        char* pass = getRowValue(dbOp.rows, 0, 2, dbOp.rowCount);
        if(strcmp(pass, user.password) == 0){
            clearBuffer(&buffer);
            return 1;
        }
    }
    clearBuffer(&buffer);
    return 0;
}



int main() {

    printIntroText();
    int setup = initialize();
    NodeList tableList = loadTables();
    Node *userTable = getNodeFromList(&tableList, "user");
    if (setup == 0) {
        createUser(userTable);
    }
    while (1) {
        printf("Login to your account\n");
        User user = getUserInfo(0);
        int auth = authenticate(user, userTable);
        if (auth == 1) {
            printSuccess("Logged in successfully");
            break;
        } else if (auth == -1) {
            printError("User doesn't exist");
        } else {
            printError("User password doesn't match");
        }
    }
    while (1) {
        printf("\n$>> ");
        char *input = handleInput();
        if (input != NULL) {
            if (caseInsensitiveCompare(input, "quit;") == 0) {
                exit(0);
            } else if (caseInsensitiveCompare(input, "create user;") == 0) {
                createUser(userTable);
            } else if (caseInsensitiveCompare(input, "list tables;") == 0) {
                printTables(tableList);
                tableList = loadTables();
            } else {
                DBOp dbOp = execSQL(input, &tableList);
                if (dbOp.code == SUCCESS) {
                    printSuccess("%s", dbOp.successMsg);
                    if(isSelectKeyword(dbOp.action) || isInsertKeyword(dbOp.action)){
                        printDbOp(&dbOp);
                    }

                } else {
                    printError("%s", dbOp.error);
                }
                clearDBOp(&dbOp);
                fflush(stdin);
                printf(" ");
            }

        }

    }
}

