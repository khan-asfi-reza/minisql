#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

int directory_exists(const char* path) {
#ifdef _WIN32
    DWORD ftyp = GetFileAttributesA(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return 0;

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return 1;

    return 0;
#else
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
#endif
}

int create_directory(const char* path) {
#ifdef _WIN32
    return CreateDirectoryA(path, NULL) || ERROR_ALREADY_EXISTS == GetLastError();
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

/**
 * Checks if a file exists in current path
 * @param filename Name of the file
 */
int fileExists(char* filename){
    // Open file to check if the file exists
    FILE *file = NULL;
    file = fopen(filename, "r");
    // If file doesn't exist it will have a null pointer
    if(file != NULL){
        fclose(file);
        return 1;
    }
    return 0;
}



/**
 * Opens a file given a filename, and write in the file
 * @param filename Name of the file
 * @param text Text to write in the file
 * @returns 0 if write failed and 1 if file write successful
 */
int createFile(char *fileName){
    FILE *file = NULL;
    file = fopen(fileName, "w");
    // If file doesn't exist it will have a null pointer
    if(file != NULL){
        // File created
        fclose(file);
        return 1;
    }
    return 0;
}



/**
 * Opens a file given a filename, and write in the file
 * @param filename Name of the file
 * @param text Text to write in the file
 * @returns 0 if write failed and 1 if file write successful
 */
int writeInFile(char *fileName, char* text){
    FILE *file = NULL;
    file = fopen(fileName, "w");
    // If file doesn't exist it will have a null pointer
    if(file != NULL){
        // Write in file
        fprintf(file, "%s", text);
        fclose(file);
        return 1;
    }
    return 0;
}


/**
 * Opens a file given a filename, and appends in the file
 * @param filename Name of the file
 * @param text Text to write in the file
 * @returns 0 if write failed and 1 if file write successful
 */
int appendInFile(char *fileName, char* text){
    FILE *file = NULL;
    file = fopen(fileName, "a");
    // If file doesn't exist it will have a null pointer
    if(file != NULL){
        // Write in file
        fprintf(file, "%s", text);
        fclose(file);
        return 1;
    }
    return 0;
}

/**
 * Opens a file given a filename, and performs ``fputs`` in the file
 * @param filename Name of the file
 * @param text Text to write in the file
 * @returns 0 if write failed and 1 if file write successful
 */
int putInFile(char *fileName, char* text, int addNewLine){
    FILE *file = NULL;
    file = fopen(fileName, "a");
    // If file doesn't exist it will have a null pointer
    if(file != NULL){
        // Write in file
        fputs(text, file);
        if (addNewLine == 1){
            fputs("\n", file);
        }
        fclose(file);
        return 1;
    }
    return 0;
}

