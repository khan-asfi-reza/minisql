# MiniSQL 

A simple database engine

## Overview
MiniSQL is a domain-specific language designed for managing data in a relational database management system (RDBMS). It is ideal for handling structured data with relations between different entities or variables. Unlike more complex relational databases like Postgres and MySQL, MiniSQL offers a simplified environment, similar to a spreadsheet but with enhanced support for structured data.

## Core Functionalities
- **User Account Management**: Create and manage user credentials.
- **User Authentication**: Control database access through a login system.
- **SQL Operations**: Support for basic SQL commands for data definition, manipulation, and querying.

## System Prerequisites
- C development environment (gcc)
- Makefile / Cmake build tool
- File system access for data storage

## Setup

### Using Makefile

```shell
make
./build/minisql
```


### Without Makefile
```shell
gcc  -c src/const.c -o build/const.o
gcc  -c src/database.c -o build/database.o
gcc  -c src/filesystem.c -o build/filesystem.o
gcc  -c src/io.c -o build/io.o
gcc  -c src/lexer.c -o build/lexer.o
gcc  -c src/main.c -o build/main.o
gcc  -c src/util.c -o build/util.o
gcc  -o build/minisql build/const.o build/database.o build/filesystem.o build/io.o build/lexer.o build/main.o build/util.o
```

It will compile the project and create build/minisql


## User manual

### Create Table

Let’s assume the user wants to hold student data, for that a table students have to be created with relevant column

```sql
CREATE TABLE students (
id INTEGER,
first_name VARCHAR,
last_name VARCHAR,
birth_date DATE,
major VARCHAR,
data_created DATE DEFAULT NOW
);
```



### Insert Data

Now to add a student record in the student table, the user must perform an INSERT query

```sql
INSERT INTO students (first_name, last_name, birth_date, major) VALUES ('Fateh','Saad', '2001-04-12', 'Computer Science');
```

The above query will insert the above record in the table. The column `id` is an automatically generated field, it will create a serial number in the record. For the very  first record the id will be 1 and data_created is also a automatically generated field, it will automatically add current date in the record

### Select Data

To retrieve information about a student with a specific id:

```sql
SELECT * FROM students WHERE id = 1;
```


To find all students with a major in 'Computer Science':

```sql
SELECT * FROM students WHERE major = 'Computer Science';
```

To combine conditions, such as finding a 'Computer Science' major named ‘ ‘Saad’:

```sql
SELECT * FROM students WHERE last_name = 'Saad' AND major = 'Computer Science';
```

### Update Data

If a student changes their major, you would update their record in the students table:
```sql
UPDATE students SET major = 'Electrical Engineering' WHERE id = 1;
```

### Delete Data

If a student record needs to be removed from the students table:

```sql
DELETE FROM students WHERE id = 1;
```


Workflow

MiniSQL sql engine’s workflow is divided into several parts. The first part is lexical analysis of an SQL command.
An SQL Command is parsed into a series/array of tokens identifying the type of the tokens.
Example:


SELECT * FROM user WHERE id = 1 AND username = ‘khan’;


The function responsible for lexical analysis will parse this command and create the following array of tokens.
```
SELECT       → Token , Token Type = Keyword
*                   → Token , Token Type = Symbol
FROM          →  Token , Token Type = Keyword
user             → Token, Token Type = Identifier
WHERE       → Token, Token Type = Keyword
id                  → Token, Token Type = Identifier
=                   → Token, Token Type = Symbol
1                   → Token, Token Type = Number
AND              → Token, Token Type = Keyword
username     → Token, Token Type = Identifier
=                   → Token, Token Type = Symbol
‘khan’            → Token, Token Type = String
```
After creating the array of tokens, the tokens array will be used in the function that generates a ASTNode, ASTNode stands for Abstract Syntax Tree Node, tough the name is Tree, but for simplicity, the data structure of this Node is not a tree based structure, rather has several array pointers that points to useful Tokens in order to perform an sql query.
From the list of tokens, it will first get the sql action command, ( SELECT, UPDATE, CREATE, DELETE, INSERT), after getting the action, it will slowly parse the tokens to find out columns, their respective data type, if the action is insert then their respective data, and filter query columns and data.
After generating the Node, the node will be passed into an sql execution function, based on the action it will perform the query at the file system level.
If the query is a `create table` query, it will create an sql file where the sql command will be stored for future reference of the table, in future for performing other queries, the reference of column and data type is required. Then there will be another file that will store the data or row records upon insertion. A file to store the primary key serial number will also be generated and a .table config file will be generated to keep track of the sql files. When the minisql instance will boot up, the minisql instance will read from the .table file to get the table details and keep them in memory.

### Additional Commands

To create a user for the minisql system
```sql
CREATE USER;
```

It will ask for a username and password and store it in the user table.

To check the list of tables.
```
LIST TABLES;
```


### Security Considerations

Passwords should be hashed and possibly salted to enhance security.
The engine handles basic textual data and does not include encryption for data in transit or at rest.
It is advisable not to use this system for sensitive data unless additional security measures are implemented.


### Conclusion

This project provides a minimalistic SQL engine, serving as an introduction to database management concepts and SQL operations. It's a tool designed for learning and lightweight application development, not intended for high-security or production environments. Users are encouraged to expand on this foundation to create more sophisticated and secure database systems.
