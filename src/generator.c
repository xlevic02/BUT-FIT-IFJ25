// Implementace generatoru vysledneho kodu IFJcode25
//generator.c by Marek "xbalism00" Bališ on 11/11/25

#include "generator.h"
#include "ast.h"
#include "scanner.h"
#include "symtable.h"
#include "error.h"

////
// Warning: this generator is working with the old version of ast.c, so there are things that I plan to change. 
// Just wanted to get this out so you can check this over before the meeting.
// And if you're asking why I didn't change it alredy when I said it should be easy?
// Turns out, it isn't. I'll have to rewrite what my generator checks while going through ast and propably merge generate_node and generate_expression which will be fun(not)
// BUT this SHOULD be able to generate assembler code. I'll be testing it when I rewrite it this friday
////

//Start of this shitshow of a program, will propably move this into generate_node
int generate_code(ast_node_ptr root, bst_scope_ptr symtable)
    {
        printf(".IFJcode25\n");
        print_jump("$$main\n\n");
        int num_of_children = root->value.int_value;
        for(int i = 0; i < num_of_children; i++)    
            {
                if(root->children[i]->token.type == TT_KEYWORD_STATIC)
                    {
                        generate_node(root->children[i], &symtable);
                    }
            }
        print_label("$$main");
        printf("CREATEFRAME\n");
        printf("PUSHFRAME\n");
        for(int i = 0; i < num_of_children; i++)
            {
                if(root->children[i]->token.type != TT_KEYWORD_STATIC)
                    {
                        generate_node(root->children[i], &symtable);
                    }
            }
        printf("POPFRAME\n");
        print_exit("int@0");
        return 0;
    }

void generate_node(ast_node_ptr node, bst_scope_ptr *current_scope)
    {
        if(node == NULL)
            {
                //If generate_node reached the end of ast, recursively return;
                return;
            }
        //Looking at the node's token type
        switch(node->token.type)
            {
                //If it's a function
                case TT_KEYWORD_STATIC:
                    {   
                        //We print the label of the function, push the alredy existing frame and go through the body
                        print_label(node->token.lexeme);
                        printf("PUSHFRAME\n");
                        generate_node(node->children[1], current_scope);
                        printf("POPFRAME\n");
                        printf("RETURN\n");
                        break;
                    }
                //The body of a funcion, if/else or while
                case TT_LBRACE:
                    {
                        //Increasing the scope
                        bst_increase_scope(current_scope);
                        //If the node's parent is a function
                        if (node->parent && node->parent->token.type == TT_KEYWORD_STATIC)
                        {
                            //Get all of the functions parameters and define them
                            ast_node_ptr parameters = node->parent->children[0];
                            for(int i = 0; i < parameters->value.int_value; i++)
                            {
                                char* var_name = get_variable_id(*current_scope, parameters->children[i]->token.lexeme);
                                char* frame = get_variable_frame(*current_scope, var_name);
                                char var_full[100];
                                sprintf(var_full, "%s@%s", frame, var_name);
                                print_defvar(var_full);
                                char parameter_reg[20];
                                sprintf(parameter_reg, "LF@%%%d", i + 1);
                                print_move(var_full, parameter_reg);
                            }
                        }
                        //Then go through the body
                        for(int i = 0; i < node->value.int_value; i++)
                            {
                                generate_node(node->children[i], current_scope);
                            }
                        bst_decrease_scope(current_scope);
                        break;
                    }
                //When it's an IF
                case TT_KEYWORD_IF:
                    {
                        //We get a label for the end and for else
                        char label_end[32];
                        char label_else[32];
                        get_unique_label(label_end, "if_end");
                        get_unique_label(label_else, "if_else");
                        label_stack_top++;
                        strcpy(label_stack[label_stack_top], label_else);
                        label_stack_top++;
                        strcpy(label_stack[label_stack_top], label_end);
                        //Then generate the conditions and check if they're true or false
                        generate_expression(node->children[0], current_scope);
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jumpifeqs("if_else");
                        //Then generate the body
                        generate_node(node->children[1], current_scope);
                        print_jump(label_end);
                        print_label(label_else);
                        break;
                    }
                //For keyword else
                case TT_KEYWORD_ELSE:
                    {
                        /*if(label_stack_top < 1)
                            {
                            //I don't know if xlevic02 chcecks this so for now I'll leave this here
                                error(2, MYSSING_TOKEN_ORDER)
                                return;
                            }*/
                        //Copy label_end from label_stack
                        char label_end[32];
                        strcpy(label_end, label_stack[label_stack_top--]);
                        label_stack_top--;
                        //And generate the body
                        generate_node(node->children[0], current_scope);
                        print_label(label_end);
                        break;
                    }
                //And now for while
                case TT_KEYWORD_WHILE:
                    {
                        //Get label for start and end
                        char w_start[32];
                        char w_end[32];
                        get_unique_label(w_start, "while_start");
                        get_unique_label(w_end, "while_end");
                        //We start the while, generate the conditions AND THEN check if ther're true
                        print_label(w_start);
                        generate_expression(node->children[0], current_scope);
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jumpifeqs(w_end);
                        //If theyr're true we generate the body and jump back to start
                        generate_node(node->children[1], current_scope);
                        print_jump(w_start);
                        //If not, we end the while
                        print_label(w_end);
                        break;
                    }
                //Return
                case TT_KEYWORD_RETURN:
                    {
                        //We get retval and check if the node has any children
                        char retval_reg[20];
                        sprintf(retval_reg, "%s@%%retval", get_variable_frame(*current_scope, "%retval"));
                        if(node->children != NULL && node->children[0] != NULL)
                            {
                                generate_expression(node->children[0], current_scope);
                                print_pops(retval_reg);
                            }
                        printf("RETURN\n");
                        break;
                    }
                //Now for variables
                case TT_KEYWORD_VAR:
                    {
                        //We get the variables name
                        char* var_name = get_variable_id(*current_scope, node->token.lexeme);
                        char var_full[100];
                        //We add the current frame to it
                        sprintf(var_full, "%s@%s", get_variable_frame(current_scope, var_name), var_name);
                        //Then define it
                        print_defvar(var_full);
                        break;
                    }
                //For anything else
                default:
                    {
                        if(ast_get_precedence(node->token.type) > 0 || node->token.type == TT_IDENTIFIER || node->token.type == TT_INT)
                            {
                                generate_expression(node, current_scope);
                            }
                        break;
                    }
                
            }
    }

//Now to generate the expresions themselves
void generate_expression(ast_node_ptr node, bst_scope_ptr *current_scope)
    {
        if(node == NULL) 
            {
                //Recursively return if we've reached the end
                return;
            }
        //Get the tokens type
        token_type_t type = node->token.type;
        //Then check what it is
        //The first five ifs are basicaly what to push
        if(type == TT_INT)
            {
                print_pushs(LITERAL_INT, node->token.lexeme, NULL);
            }
        else if(type == TT_FLOAT)
            {
                print_pushs(LITERAL_FLOAT, format_float_for_ifjcode(node->token.lexeme), NULL);
            }
        else if(type == TT_STRING)
            {
                print_pushs(LITERAL_STRING, format_string_for_ifjcode(node->token.lexeme), NULL);
            }
        else if(type == TT_KEYWORD_Null)
            {
                print_pushs(LITERAL_NULL, "nil", NULL);
            }
        else if(type == TT_IDENTIFIER)
            {
                print_pushs(VARIABLE, node->token.lexeme, get_variable_frame(*current_scope, node->token.lexeme));
            }
        //Then there's the plus
        else if(type == TT_PLUS)
            {
                //Here we have to check what the childrens token.type is, so we know if we generate ADDS or CONCAT
                token_type_t int_float_or_string_0 = node->children[0]->token.type, int_float_or_string_1 = node->children[1]->token.type;
                if((int_float_or_string_0 == TT_INT && int_float_or_string_1 == TT_INT) || (int_float_or_string_0 == TT_FLOAT && int_float_or_string_1 == TT_FLOAT))
                    {
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_arithmetic(ADD);
                    }
                else
                    {
                        //If we generate CONCAT (god why), we have to create additional strings so that we can POPS the strings in
                        //the stack into them
                        char str1_name[32], str2_name[32], res_name[32];
                        char str1_full[40], str2_full[40], res_full[40];
                        int unique_id = label_counter++; 
                        sprintf(str1_name, "%%concat_str1$%d", unique_id);
                        sprintf(str2_name, "%%concat_str2$%d", unique_id);
                        sprintf(res_name, "%%concat_res$%d", unique_id);
                        sprintf(str1_full, "LF@%s", str1_name);
                        sprintf(str2_full, "LF@%s", str2_name);
                        sprintf(res_full, "LF@%s", res_name);
                        print_defvar(str1_full); 
                        print_defvar(str2_full); 
                        print_defvar(res_full);
                        //THEN we can generate the node's children themselves, and pop them into the additional ones
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_pops(str2_full);
                        print_pops(str1_full);
                        //And now (finally) we print CONCAT, and push the result into the stack
                        print_concat(res_full, str1_full, str2_full); 
                        print_pushs(VARIABLE, res_name, "LF");
                    }
            }
        //Minus and mul should be fine
        else if(type == TT_MINUS)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_arithmetic(SUB);
            }
        else if(type == TT_MUL)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_arithmetic(MUL);
            }
        //In here we first check if we're dividing by zero;
        else if(type == TT_DIV)
            {
                if(node->children[0]->token.lexeme == 0 || node->children[1]->token.lexeme == 0)
                    {
                        //If yes, error
                        error(57, ERROR_DIVISION_BY_ZERO);
                    }
                //Then we have to check if we're dividing ints or floats, because they have different DIV
                token_type_t int_or_float_0 = node->children[0]->token.type, int_or_float_1 = node->children[1]->token.type;
                if(int_or_float_0 == TT_FLOAT && int_or_float_1 == TT_FLOAT)
                    {
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_arithmetic(DIV);
                    }
                else
                    {
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_arithmetic(IDIV);
                    }
            }
        //All of relation types (I hope)
        else if(type == TT_GT)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_relation(GT);
            }
        else if(type == TT_LT)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_relation(LT);
            }
        else if(type == TT_EQ)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_relation(EQ);
            }
        else if(type == TT_LE)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_relation(GT);
                print_boolean(NOT);
            }
        else if(type == TT_GE)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_relation(LT);
                print_boolean(NOT);
            }
        else if(type == TT_NEQ)
            {
                generate_expression(node->children[0], current_scope);
                generate_expression(node->children[1], current_scope);
                print_relation(EQ);
                print_boolean(NOT);
            }
        //And here's keyword_IFJ, which I'll most likely move to generate_node this, weekend
        else if(type == TT_KEYWORD_IFJ)
            {
                //We basicaly get the tokens lexeme and see what there so we know what to generate
                char *func_name = node->token.lexeme;
                if(strcmp(func_name, "write") == 0)
                    {
                        //WRITE
                        for(int i = 0; i < node->value.int_value; i++)
                            {
                                char temp_var[32];
                                sprintf(temp_var, "LF@%%temp_write%d", label_counter++);
                                print_defvar(temp_var);
                                print_pops(temp_var);
                                print_write(temp_var);
                            }
                        print_pushs(LITERAL_NULL, "nil", NULL);
                    }
                else if(strcmp(func_name, "read_str") == 0)
                    {
                        //READ_STR
                        char temp_var[32];
                        sprintf(temp_var, "LF@%%temp_read%d", label_counter++);
                        print_defvar(temp_var);
                        print_read("string", temp_var);
                        print_pushs(VARIABLE, temp_var, "LF");
                    }
                else if(strcmp(func_name, "read_num") == 0)
                    {
                        //READ_NUM
                        char temp_var[32];
                        sprintf(temp_var, "LF@%%temp_read%d", label_counter++);
                        print_defvar(temp_var);
                        print_read("int", temp_var);
                        print_pushs(VARIABLE, temp_var, "LF");
                    }
                else if(strcmp(func_name, "str") == 0)
                    {
                        //INT2STR, TODO FLOAT2STR/
                        generate_expression(node->children[0], current_scope);
                        print_conversion(INT2STR);
                    }
                else if(strcmp(func_name, "floor") == 0)
                    {
                        //FLOAT2INT
                        generate_expression(node->children[0], current_scope);
                        print_conversion(FLOAT2INT);
                    }
                else if(strcmp(func_name, "lenght") == 0)
                    {
                        //STRLEN, we basicaly create additional variables to get the string from stack and use strlen
                        char str_var[32], res_var[32];
                        sprintf(str_var, "LF@%%len_str%d", label_counter);
                        sprintf(res_var, "LF@%%len_res%d", label_counter++);
                        print_defvar(str_var);
                        print_defvar(res_var);
                        generate_expression(node->children[0], current_scope);
                        print_pops(str_var);
                        print_strlen(res_var, str_var);
                        print_pushs(VARIABLE, res_var, "LF");
                    }
                else if(strcmp(func_name, "substring") == 0)
                    {
                                //TODO
                                //Tried to do this, failed spectacularly, will try again after the exam, now I really don't have the brain capacity to do this
                    }
                else if(strcmp(func_name, "strcmp") == 0)
                    {
                        //TODO STRLEN1 - STRLEN2
                        //Almost done, now I'll just have to create some additional variables to help, some jumps
                        // and the labels for those jumps
                        char str1_var[20], str2_var[20];
                        print_pops(str1_var);
                        print_strlen(str1_var, node->children[0]->token.lexeme);
                        print_pops(str2_var);
                        print_strlen(str2_var, node->children[1]->token.lexeme);
                        print_pushs(LITERAL_INT, str2_var, NULL);
                        print_pushs(LITERAL_INT, str1_var, NULL);
                        print_arithmetic(SUB);
                    }
                else if(strcmp(func_name, "ord") == 0)
                    {
                        //STRI2INT
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_conversion(STRI2INT);
                    }
                else if(strcmp(func_name, "chr") == 0)
                    {
                        //INT2CHAR
                        generate_expression(node->children[0], current_scope);
                        print_conversion(INT2CHAR);
                    }
            
            }
            
    }

//Get the frame of the variable
//Will most likely change this
char *get_variable_frame(bst_scope_ptr scope, char* var_name)
    {
        (void)var_name;
        if(scope->parent == NULL)
            {
                return "GF";
            }
        else
            {
                return "LF";
            }
    }

//Gets the ID
//Will change this too
char* get_variable_id(bst_scope_ptr scope, char* lexeme)
    {
        (void)scope;
        return lexeme;
    }

//Gives a unique label
void get_unique_label(char* buffer, const char* prefix)
    {
        sprintf(buffer, "$$%s$%d", prefix, label_counter++);
    }

//Formats float for the final code
char* format_float_for_ifjcode(const char* lexeme)
    {
        static char g_float_buffer[100];
        sprintf(g_float_buffer, "%a", strtod(lexeme, NULL));
        return g_float_buffer;
    }

//Formats string for final code
char* format_string_for_ifjcode(const char* lexeme)
    {
        static char g_string_buffer[1024];
        char* buf_ptr = g_string_buffer;
        int lexeme_len = strlen(lexeme);
        for (int i = 0; i < lexeme_len; i++)
        {
            unsigned char c = lexeme[i];
            if (c <= 32 || c == 35 || c == 92) 
            {
                sprintf(buf_ptr, "\\%03d", (int)c);
                buf_ptr += 4;
            }
            else
            {
                *buf_ptr = c;
                buf_ptr++;
            }
        }
        *buf_ptr = '\0';
        return g_string_buffer;
    }

void print_call(char* label)
    {
        printf("CALL %s\n", label);
    }

void print_defvar(char* var_name)
    {
        printf("DEFVAR %s\n", var_name);
    }

void print_move( char* destination , char* source)
    {
        printf("MOVE %s %s\n", destination, source);
    }

void print_pushs(SYM_TYPE type, char* value, char* frame)
    {
        switch(type)
            {
                case VARIABLE:
                    {
                        printf("PUSHS %s@%s\n", frame, value);
                        break;
                    }
                case LITERAL_INT:
                    {
                        printf("PUSHS int@%s\n", value);
                        break;
                    }
                case LITERAL_FLOAT:
                    {
                        printf("PUSHS float@%s\n", value);
                        break;
                    }
                case LITERAL_STRING:
                    {
                        printf("PUSHS string@%s\n", value);
                        break;
                    }
                case LITERAL_BOOL:
                    {
                        printf("PUSHS bool@%s\n", value);
                        break;
                    }
                case LITERAL_NULL:
                    {
                        printf("PUSHS nil@nil\n");
                        break;
                    }
            }
    }
void print_pops(char* var)
    {
        printf("POPS %s\n", var);
    }

void print_clears()
    {
        printf("CLEARS\n");
    }

void print_arithmetic(ARITHMETIC arithmetic)
    {
        switch(arithmetic)
            {
                case ADD:
                    printf("ADDS\n");
                    break;
                case SUB:
                    printf("SUBS\n");
                    break;
                case MUL:
                    printf("MULS\n");
                    break;
                case DIV:
                    printf("DIVS\n");
                    break;
                case IDIV:
                    printf("IDIVS\n");
                    break;
                default:
                    return;
            }
                
    }

void print_relation(RELATION relation)
    {
        switch(relation)
            {
                case LT:
                    printf("LTS\n");
                    break;
                case GT:
                    printf("GTS\n");
                    break;
                case EQ:
                    printf("EQS\n");
                    break;
                default:
                    return;
            }
    }

void print_boolean(BOOLEAN boolean)
    {
        switch(boolean)
            {
                case AND:
                    printf("ANDS\n");
                    break;
                case OR:
                    printf("ORS\n");
                    break;
                case NOT:
                    printf("NOTS\n");
                    break;
                default:
                    return;
            }
    }

void print_conversion(CONVERSION conversion)
    {
        switch(conversion)
            {
                case INT2FLOAT:
                    printf("INT2FLOATS\n");
                    break;
                case FLOAT2INT:
                    printf("FLOAT2INTS\n");
                    break;
                case INT2CHAR:
                    printf("INT2CHARS\n");
                    break;
                case STRI2INT:
                    printf("STRI2INTS\n");
                    break;
                case FLOAT2STR:
                    printf("FLOAT2STRS\n");
                    break;
                case INT2STR:
                    printf("INT2STRS\n");
                    break;
                default:
                    return;
            }
    }

void print_read(char* type, char* var)
    {
        printf("READ %s %s\n", var, type);
    }

void print_write(char* symbol)
    {
        printf("WRITE %s\n", symbol);
    }

void print_concat(char* var, char* symbol_1, char* symbol_2)
    {
        printf("CONCAT %s %s %s\n", var, symbol_1, symbol_2);
    }

void print_strlen(char* var, char* symbol)
    {
        printf("STRLEN %s %s\n", var, symbol);
    }

void print_getchar(char* var, char* symbol, char* index)
    {
        printf("GETCHAR %s %s %s\n", var, symbol, index);
    }

void print_setchar(char* var, char* index, char* symbol)
    {
        printf("SETCHAR %s %s %s\n", var, index, symbol);
    }

void print_types()
    {
        printf("TYPES\n");
    }

void print_isint_isints()
    {
        printf("ISINTS\n");
    }

void print_label(char* label)
    {
        printf("LABEL %s\n", label);
    }

void print_jump(char* label)
    {
        printf("JUMP %s\n", label);
    }

void print_jumpifeqs(char* label)
    {
        printf("JUMPIFEQS %s\n", label);
    }

void print_jumpifneqs(char* label)
    {
        printf("JUMPIFNEQS %s\n", label);
    }

void print_exit(char* symbol)
    {
        printf("EXIT %s\n", symbol);
    }

void print_break()
    {
        printf("BREAK\n");
    }
void print_dprint(char* symbol)
    {
        printf("DPRINT %s\n", symbol);
    }
