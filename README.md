# BUT-FIT-IFJ25

Project name:
Implementace prekladace imperativniho jazyka IFJ25. (vv-BVS)


Team members:
- Jan Frantisek "xlevic02" Levicek (leader)
- Jan "xspacej00" Spacek
- Marek "xbalism00" Balis
- William Denis "xtihelw00" Tihelka
---------------------------------------------------------------------------

Head of every file must start with:

// Implementace prekladace imperativniho jazyka IFJ25
// FILE_NAME by FIRST_NAME (SECOND_NAME) "xlogin00" LAST_NAME on MM/DD/25.
//

Naming convention:

1. Variable names:
    - use snake_case
    - widely used variables should be named the same across functions
2. Constant names:
    - use UPPER_SNAKE_CASE
    - start with type of the constant
        example:
            * TT_STRING (token type string)
            * ERROR_INTERNAL (error code - internal error)
            * MSG_SYN_MISSING_TOKEN (error message: syntax error - missing token)
3. Function names:
    - use snake_case
    - start with file name
        example:
            * token_delete_list (token.c - delete list)
4. Struct, enum and typedef names:
    - use snake_case
    - end with _t
        example:
            * token_type_t
5. File names:
    - use snake_case
    - headers should be named same as .c files
6. Header guards:
    - use UPPER_SNAKE_CASE
    - starts with BUT_FIT_IFJ25_
    - ends with _H
        example:
            * BUT_FIT_IFJ25_TOKEN_H
7. Error messages:
    - frequently used messages should have a macro
    - use UPPER_SNAKE_CASE
    - starts with MSG_ + (3 letters describing the error type) + (error message)
        example:
            * MSG_SYN_MISSING_TOKEN (message: syntax error - missing token)
    - message should have format "(type of error):\t(error message)\n"
        example:
            * "Syntax error:\tmissing token\n"
