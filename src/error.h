// Implementace prekladace imperativniho jazyka IFJ24
// Error by Jan Frantisek "xlevic02" Levicek on 10/3/24.
//

#ifndef BUT_FIT_IFJ25_ERROR_H
#define BUT_FIT_IFJ25_ERROR_H


// Error codes
#define ERROR_LEXICAL 1                        // lexical error
#define ERROR_SYNTAX 2                         // syntax error
#define ERROR_SEM_UNDEF 3                      // semantic error: undefined function or variable
#define ERROR_SEM_REDEF 4                      // semantic error: redefinition of function or variable; assigning value to unmodifiable variable
#define ERROR_SEM_STATIC_PARAM_NUM_OR_TYPE 5   // static semantic error: wrong number of parameters; wrong type of parameters in built-in function
#define ERROR_SEM_STATIC_COMPATIBILITY 6       // static semantic error: compatibility error in arithmetic, string or relational expression
#define ERROR_SEM_OTHER 10                     // other semantic error
#define ERROR_SEM_RUNNING_PARAM_TYPE 25        // running semantic error: wrong type of parameters in built-in function
#define ERROR_SEM_RUNNING_COMPATIBILITY 26     // running semantic error: compatibility error in arithmetic, string of relational expression
#define ERROR_INTERNAL 99                      // compilers internal error


// Generator error codes
#define ERROR_INVALID_INPUT_PARAMS 50   // runtime error: incorrect input parameters specified on command line when running the interpreter
#define ERROR_ANALYSIS_FAILURE 51       // error during the analysis (lexical, syntactic) of IFJcode25 source code
#define ERROR_SEMANTIC_CHECK_FAILURE 52 // error during semantic checks of IFJcode25 source code
#define ERROR_RUNTIME_WRONG_OPERAND 53  // runtime error: incorrect operand types during interpretation
#define ERROR_ACCESS_NONEXISTENT_VAR 54 // runtime error: access to a non-existent variable (frame exists)
#define ERROR_NONEXISTENT_FRAME 55      // runtime error: frame does not exist (e.g., reading from empty stack of frames)
#define ERROR_MISSING_VALUE 56          // runtime error: missing value in variable, data stack, or call stack
#define ERROR_DIVISION_BY_ZERO 57       // runtime error: incorrect operand value (e.g., division by zero, incorrect EXIT return value)
#define ERROR_STRING_MANIPULATION 58    // runtime error: incorrect manipulation of a string
#define ERROR_GEN_INTERNAL 60           // internal error of interpreter not influenced by input program (e.g., memory allocation failure, file opening error)

// Lexical error
#define MSG_LEX_PROHIBITED_CHAR "Lexical error:\tProhibited or unexpected character\n"
#define MSG_LEX_INCOMPLETE_TOKEN "Lexical error:\tIncomplete token\n"
#define MSG_LEX_UNCLOSED_STRING "Lexical error:\tUnclosed string literal.\n"
#define MSG_LEX_UNCLOSED_COMMENT "Lexical error:\tUnclosed block comment (/* ... missing */).\n"
#define MSG_LEX_INVALID_NUMBER "Lexical error:\tInvalid numeric literal format.\n"
#define MSG_LEX_INVALID_ESCAPE "Lexical error:\tInvalid escape sequence in string.\n"

// Syntax error
#define MSG_SYN_MISSING_EOL "Syntax error:\tmissing end of line\n"
#define MSG_SYN_UNEXPECTED_EOF "Syntax error:\tunexpected end of file\n"
#define MSG_SYN_MISSING_TOKEN "Syntax error:\tmissing token\n"
#define MSG_SYN_MISSING_IDENTIFIER "Syntax error:\tmissing identifier\n"
#define MSG_SYN_TOKEN_ORDER "Syntax error:\twrong token order\n"
#define MSG_SYN_EXPRESSION "Syntax error:\tExpression or literal/numeral wrong\n"
#define MSG_SYN_MISSING_PROLOG "Syntax error:\tmissing prolog\n"
#define MSG_SYN_MISSING_MAIN "Syntax error:\tmissing main\n"
#define MSG_SYN_MISSING_CLASS "Syntax error:\tmissing class\n"
#define MSG_SYN_PROGRAM_DECLARATION "Syntax error:\tincorrect Program declaration\n"

// Semantic error
#define MSG_SEN_UNDEFINED_VAR "Semantic error:\tuse of undefined variable/function\n"
#define MSG_SEN_REDEFINING "Semantic error:\tredefining variable/function\n"
#define MSG_SEN_INCOMPATIBLE "Semantic error:\twrong type of variable input into a function\n"

// Internal error
#define MSG_INT_MALLOC "Internal error:\tfailed to allocate memory for a new token\n"
#define MSG_INT_DSTR_APPEND "Internal error:\tfailed to append buffer dynamic string\n"
#define MSG_INT_DSTR_RESET "Internal error:\tfailed to reallocate memory for buffer dynamic string\n"
#define MSG_INT_DSTR_INIT "Internal error:\tfailed to initialize buffer dynamic string\n"
#define MSG_INT_MISSING_TOKEN "Internal error:\tfailed to pass along tokens properly\n"
#define MSG_INT_REALLOC "Internal error:\tfailed to reallocate memory\n"

// Generator error msg
#define MSG_INVALID_INPUT_PARAMS "Runtime error: \tincorrect input parameters specified on command line when running the interpreter\n"
#define MSG_ANALYSIS_FAILURE "Error during the analysis (lexical, syntactic) of IFJcode25 source code\n"
#define MSG_SEMANTIC_CHECK_FAILURE "Error during semantic checks of IFJcode25 source code\n"
#define MSG_RUNTIME_WRONG_OPERAND "Runtime error:\tincorrect operand types during interpretation\n"
#define MSG_ACCESS_NONEXISTENT_VAR "Runtime error:\taccess to a non-existent variable (frame exists)\n"
#define MSG_NONEXISTENT_FRAME "Runtime error:\tframe does not exist (e.g., reading from empty stack of frames)\n"
#define MSG_MISSING_VALUE "Runtime error:\tmissing value in variable, data stack, or call stack\n"
#define MSG_DIVISION_BY_ZERO "Runtime error:\tincorrect operand value (e.g., division by zero, incorrect EXIT return value)\n"
#define MSG_STRING_MANIPULATION "Runtime error:\tincorrect manipulation of a string\n"
#define MSG_GEN_INTERNAL "Internal error of interpreter not influenced by input program (e.g., memory allocation failure, file opening error)\n"

void error(int err_num, const char* err_message);

#endif //BUT_FIT_IFJ25_ERROR_H
