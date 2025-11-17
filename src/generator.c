// Implementace generatoru vysledneho kodu IFJcode25
//generator.c by Marek "xbalism00" Bališ on 11/11/25

#include "generator.h"
//#include "ast.h"
#include "scanner.h"
//#include "symtable.h"
#include "error.h"

////
// Warning: this generator is working with the old version of ast.c.
// BUT this SHOULD be able to generate assembler code.
////

//Start of this shitshow of a program
//I get the root node and symtable
int generate_code(ast_node_ptr root, bst_scope_ptr symtable)
    {
        printf(".IFJcode25\n");
        print_jump("$$main\n\n");
        //I assign roots number of children to int nu_of children
        int num_of_children = root->n_of_children;
        for(int i = 0; i < num_of_children; i++)    
            {
                //Generate all the functions that are children of root
                if(root->children[i]->token.type == TT_KEYWORD_STATIC)
                    {
                        generate_node(root->children[i], &symtable);
                    }
            }
        //Start main
        print_label("$$main");
        printf("CREATEFRAME\n");
        printf("PUSHFRAME\n");
        for(int i = 0; i < num_of_children; i++)
            {
                //Generate all the other children
                if(root->children[i]->token.type != TT_KEYWORD_STATIC)
                    {
                        generate_node(root->children[i], &symtable);
                    }
            }
        printf("POPFRAME\n");
        print_exit("int@0");
        return 0;
    }

//I generate the nodes themselves
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
                        //Increasing the scope
                        bst_increase_scope(current_scope);
                        //If the node's parent is a function
                        //Get all of the functions parameters and define them
                        ast_node_ptr parameters = node->children[0];
                        for(int i = 0; i < parameters->n_of_children; i++)
                        {
                            char* var_name = get_variable_id(*current_scope, parameters->children[i]->token.lexeme);
                            char* frame = get_variable_frame(*current_scope, var_name);
                            char var_full[100];
                            sprintf(var_full, "%s@%s", frame, var_name);                                print_defvar(var_full);
                            char parameter_reg[20];
                            sprintf(parameter_reg, "LF@%%%d", i + 1);
                            print_move(var_full, parameter_reg);
                        }
                        for(int i = 1; i < node->n_of_children; i++)
                            {
                                generate_node(node->children[i], current_scope);
                            }
                        bst_decrease_scope(current_scope);
                        printf("POPFRAME\n");
                        printf("RETURN\n");
                        break;
                    }
                //The body of a funcion, if/else or while
                case TT_LBRACE:
                    {
                        //Increasing the scope
                        bst_increase_scope(current_scope);
                        //Go through the body
                        for(int i = 0; i < node->n_of_children; i++)
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
                        print_jumpifeqs(label_else);
                        //Then generate the body
                         //Increasing the scope
                        bst_increase_scope(current_scope);
                        //Go through the body
                        for(int i = 1; i < node->n_of_children; i++)
                            {
                                generate_node(node->children[i], current_scope);
                            }
                        bst_decrease_scope(current_scope);
                        print_jump(label_end);
                        print_label(label_else);
                        break;
                    }
                //For keyword else
                case TT_KEYWORD_ELSE:
                    {
                        //Copy label_end from label_stack
                        char label_end[32];
                        strcpy(label_end, label_stack[label_stack_top--]);
                        label_stack_top--;
                        //And generate the body
                        //Increasing the scope
                        bst_increase_scope(current_scope);
                        //Go through the body
                        for(int i = 1; i < node->n_of_children; i++)
                            {
                                generate_node(node->children[i], current_scope);
                            }
                        bst_decrease_scope(current_scope);
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
                        //Increasing the scope
                        bst_increase_scope(current_scope);
                        //Go through the body
                        for(int i = 1; i < node->n_of_children; i++)
                            {
                                generate_node(node->children[i], current_scope);
                            }
                        bst_decrease_scope(current_scope);
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
                        sprintf(var_full, "%s@%s", get_variable_frame(*current_scope, var_name), var_name);
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
                //We check if the identifier has any children
                if(node->children != NULL)
                    {
                        //If it does it's a user defined function
                        printf("CREATEFRAME\n");
                        for(int i = 0; i < node->value.int_value; i++)
                            {
                                generate_expression(node->children[i], current_scope);
                                char parameter_reg[20];
                                sprintf(parameter_reg, "TF@%%%d", i + 1);
                                print_defvar(parameter_reg);
                                print_pops(parameter_reg);
                            }
                        print_call(node->token.lexeme);
                        print_pushs(VARIABLE, "%retval", "TF");
                    }
                else 
                    {
                        //Check if it's a getter
                        token_type_t id_type = generator_get_id_type(node->token.lexeme, current_scope);
                        if(id_type == TT_KEYWORD_STATIC)
                            {
                                print_call(node->token.lexeme);
                                print_pushs(VARIABLE, "%retval", "TF");
                            }
                        else
                            {
                                //If not ve just push the variable
                                print_pushs(VARIABLE, node->token.lexeme, get_variable_frame(*current_scope, node->token.lexeme));
                            }
                    }
            }
        //Here's assign
        else if(type == TT_ASSIGN)
            {
                //We check if the first child is an identifier, if not then error
                if (node->children[0]->token.type != TT_IDENTIFIER) 
                    {
                        ast_error(ERROR_SEM_OTHER, "Error: Left side of assign is not a variable.\n", node, NULL);
                        return;
                    }
                char *var_name = node->children[0]->token.lexeme;
                char setter_name[256];
                sprintf(setter_name, "%s=", var_name);
                token_type_t setter_type = generator_get_id_type(setter_name, current_scope);
                if(setter_type != TT_KEYWORD_STATIC)
                    {
                        char *temp_var = get_variable_id(*current_scope, node->children[0]->token.lexeme);
                        char *frame = get_variable_frame(*current_scope, temp_var);
                        char var_full[100];
                        sprintf(var_full, "%s@%s", frame, temp_var);
                        //We'll generate the second child
                        generate_expression(node->children[1], current_scope);
                        //Pop it into the string
                        print_pops(var_full);
                        //And push it back into the stack
                        print_pushs(VARIABLE, temp_var, frame);
                    }
                else
                    {
                        //If node->children[0] is a setter
                        printf("CREATEFRAME\n");
                        generate_expression(node->children[1], current_scope);
                        print_defvar("TF@%1");
                        printf("POPS TF@%1\n");
                        printf("PUSHS TF@%1\n");
                        print_call(setter_name);
                    }
            }
        //Then there's plus
        else if(type == TT_PLUS)
            {
                //Here we have to check what the childrens token.type is, so we know if we generate ADDS or CONCAT
                //The If here is just a placeholder, since I didn't have access to semantic analysis
                token_type_t left_type = generator_get_type(node->children[0], current_scope);
                token_type_t right_type = generator_get_type(node->children[1], current_scope);
                if (left_type == TT_ERROR || right_type == TT_ERROR) {
                     ast_error(ERROR_SEM_UNDEF, MSG_SEN_UNDEFINED_VAR, node, NULL);
                     return;
                }
                if(left_type != TT_STRING && right_type != TT_STRING)
                    {
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_arithmetic(ADD);
                    }
                else
                    {
                        //If we generate CONCAT (god why), we have to create and define additional strings so that we can POPS the strings in
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
        else if(type == TT_DIV)
            {
                //I have to check if we're dividing ints or floats, because they have different DIV
                //The If here is just a placeholder, since I didn't have access to semantic analysis
                token_type_t left_type = generator_get_type(node->children[0], current_scope);
                token_type_t right_type = generator_get_type(node->children[1], current_scope);
                if (left_type == TT_ERROR || right_type == TT_ERROR) {
                     ast_error(ERROR_SEM_UNDEF, MSG_SEN_UNDEFINED_VAR, node, NULL);
                     return;
                }
                if(left_type == TT_FLOAT && right_type == TT_FLOAT)
                    {
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        char div_zero_check[40];
                        char div_not_zero[32];
                        sprintf(div_zero_check, "LF@%%div_check_float$%d", label_counter);
                        get_unique_label(div_not_zero, "not_zero");
                        label_counter++;
                        print_defvar(div_zero_check);
                        print_pops(div_zero_check);
                        print_pushs(VARIABLE, div_zero_check, "LF");
                        print_pushs(LITERAL_FLOAT, "0x0.0p+0", NULL);
                        print_jumpifneqs(div_not_zero);
                        print_exit("int@57");
                        print_label(div_not_zero);
                        print_arithmetic(DIV);
                    }
                else
                    {
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        char div_zero_check[40];
                        char div_not_zero[32];
                        sprintf(div_zero_check, "LF@%%div_check_float$%d", label_counter);
                        get_unique_label(div_not_zero, "not_zero");
                        label_counter++;
                        print_defvar(div_zero_check);
                        print_pops(div_zero_check);
                        print_pushs(VARIABLE, div_zero_check, "LF");
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_jumpifneqs(div_not_zero);
                        print_exit("int@57");
                        print_label(div_not_zero);
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
        else if(type == TT_KEYWORD_IS)
            {
                generate_expression(node->children[0], current_scope);
                char tmp_value[40], result_type[40];
                char label_true[32], label_end[32];
                sprintf(tmp_value, "LF@%%is_value$%d", label_counter);
                sprintf(result_type, "LF@%%result_type$%d", label_counter);
                get_unique_label(label_true, "true");
                get_unique_label(label_end, "end");
                label_counter++;
                print_defvar(tmp_value);
                print_defvar(result_type);
                print_pops(tmp_value);
                printf("TYPE %S %S\n", result_type, tmp_value);
                token_type_t expected_type = node->children[1]->token.type;
                if(expected_type == TT_KEYWORD_Null || expected_type == TT_NULL)
                    {
                        printf("JUMPIFEQ %s %s string@nil\n", label_true, result_type);
                    }
                else if(expected_type == TT_KEYWORD_NUM)
                    {
                        printf("JUMPIFEQ %s %s string@int\n", label_true, result_type);
                        printf("JUMPIFEQ %s %s string@float\n", label_true, result_type);
                    }
                else if(expected_type == TT_STRING)
                    {
                        printf("JUMPIFEQ %s %s string@string\n", label_true, label_end);
                    }
                print_pushs(LITERAL_BOOL, "false", NULL);
                print_jump(label_end);
                print_label(label_true);
                print_pushs(LITERAL_BOOL, "true", NULL);
                print_label(label_end);
            }
        //And here's keyword_IFJ, which are builtin functions
        else if(type == TT_KEYWORD_IFJ)
            {
                //We basicaly get the tokens lexeme and see whats there so we know what to generate
                char *func_name = node->token.lexeme;
                if(strcmp(func_name, "write") == 0)
                    {
                        //WRITE
                        for(int i = 0; i < node->value.int_value; i++)
                            {
                                generate_expression(node->children[i], current_scope);
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
                        //INT2STR, FLOAT2STR
                        token_type_t arg_type = generator_get_type(node->children[0], current_scope);
                        if(arg_type == TT_INT)
                            {
                                generate_expression(node->children[0], current_scope);
                                print_conversion(INT2STR);
                            }
                        else
                            {
                                generate_expression(node->children[0], current_scope);
                                print_conversion(FLOAT2STR);
                            }
                    }
                else if(strcmp(func_name, "floor") == 0)
                    {
                        //FLOAT2INT
                        generate_expression(node->children[0], current_scope);
                        print_conversion(FLOAT2INT);
                    }
                else if(strcmp(func_name, "length") == 0)
                    {
                        //STRLEN, we basicaly create additional variables to get the string from stack and use strlen
                        char str_var[32], result_var[32];
                        sprintf(str_var, "LF@%%strlen_str%d", label_counter);
                        sprintf(result_var, "LF@%%strlen_result%d", label_counter++);
                        print_defvar(str_var);
                        print_defvar(result_var);
                        generate_expression(node->children[0], current_scope);
                        print_pops(str_var);
                        print_strlen(result_var, str_var);
                        print_pushs(VARIABLE, result_var, "LF");
                    }
                else if(strcmp(func_name, "substring") == 0)
                    {
                                //Substring: Created vars to pop the arguments and the final result into
                                //Created a loop at the end to create the substring by putting the variable at index i into tmp_char and then using concat to add it to result_var
                                char str_var[32], i_var[32], j_var[32], result_var[32], tmp_char[32];
                                char loop_start[32], loop_end[32];
                                sprintf(str_var, "LF@%%substr%d", label_counter);
                                sprintf(i_var, "LF@%%substr_i%d", label_counter);
                                sprintf(j_var, "LF@%%substr_j%d", label_counter);
                                sprintf(result_var, "LF@%%substr_result%d", label_counter);
                                sprintf(tmp_char, "LF@%%substr%d", label_counter++);
                                get_unique_label(loop_start, "substr_loop_start");
                                get_unique_label(loop_end, "substr_loop_end");
                                print_defvar(str_var);
                                print_defvar(i_var);
                                print_defvar(j_var);
                                print_defvar(result_var);
                                print_defvar(tmp_char);
                                generate_expression(node->children[0], current_scope);
                                generate_expression(node->children[1], current_scope);
                                generate_expression(node->children[2], current_scope);
                                print_pops(j_var);
                                print_pops(i_var);
                                print_pops(str_var);
                                print_move(result_var, "string@");
                                print_label(loop_start);
                                print_pushs(VARIABLE, i_var, "LF");
                                print_pushs(VARIABLE, j_var, "LF");
                                print_relation(LT);
                                printf("PUSHS bool@false\n");
                                print_jumpifeqs(loop_end);
                                print_getchar(tmp_char, str_var, i_var);
                                print_concat(result_var, result_var, tmp_char);
                                printf("PUSHS %s\n", i_var);
                                printf("PUSHS int@1\n");
                                print_arithmetic(ADD);
                                print_pops(i_var);
                                print_jump(loop_start);
                                print_label(loop_end);
                                print_pushs(VARIABLE, result_var, "LF");

                    }
                else if(strcmp(func_name, "strcmp") == 0)
                    {
                        //Compare s1 with s2, and then pushes either -1, 1 or 0 depending on if s1 is shorter, longer or equal with s2
                        //Almost done, now I'll just have to create some additional variables to help, some jumps
                        // and the labels for those jumps
                        char str1_var[20], str2_var[20];
                        char label_str1_lesser[32], label_str1_greater[32], label_strcmp_end[32];
                        get_unique_label(label_str1_lesser, "strcmp_lesser");
                        get_unique_label(label_str1_greater, "strcmp_greater");
                        get_unique_label(label_strcmp_end, "strcmp_end");
                        sprintf(str1_var, "LF@strcmp_s1%d", label_counter);
                        sprintf(str2_var, "LF@strcmp_s2%d", label_counter++);
                        print_defvar(str1_var);
                        print_defvar(str2_var);
                        generate_expression(node->children[0], current_scope);
                        generate_expression(node->children[1], current_scope);
                        print_pops(str2_var);
                        print_pops(str1_var);
                        print_pushs(VARIABLE, str1_var, "LF");
                        print_pushs(VARIABLE, str2_var, "LF");
                        print_relation(LT);
                        printf("PUSHS bool@true\n");
                        print_jumpifeqs(label_str1_lesser);
                        print_pushs(VARIABLE, str1_var, "LF");
                        print_pushs(VARIABLE, str2_var, "LF");
                        print_relation(GT);
                        printf("PUSHS bool@true\n");
                        print_jumpifeqs(label_str1_greater);
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_jump(label_strcmp_end);
                        print_label(label_str1_lesser);
                        print_pushs(LITERAL_INT, "-1", NULL);
                        print_jump(label_strcmp_end);
                        print_label(label_str1_greater);
                        print_pushs(LITERAL_INT, "1", NULL);
                        print_label(label_strcmp_end);
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
                else
                    {
                        //For any other functions
                        printf("CREATEFRAME\n");
                        for(int i = 0; i < node->value.int_value; i++)
                            {
                                generate_expression(node->children[i], current_scope);
                                char parameter_reg[20];
                                sprintf(parameter_reg, "TF@%%%d", i + 1);
                                print_defvar(parameter_reg);
                                print_pops(parameter_reg);
                            }
                        print_call(func_name);
                        print_pushs(VARIABLE, "%retval", "TF");
                    }
            }
            
    }

//Get the frame of the variable
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

//
//Here are placeholder function for semantic analysis since it's not complete yet
//

token_type_t generator_get_id_type(char* lexeme, bst_scope_ptr* current_scope)
    { 
        unsigned int key = get_hash(lexeme);
        bst_node_content_t content = bst_search_scope(*current_scope, key);
        return content.type;
    }

token_type_t generator_get_type(ast_node_ptr node, bst_scope_ptr* current_scope)
    {
        if(node == NULL)    
            {
                return TT_ERROR;
            }
        if(node->token.type == TT_INT) 
            {
                return TT_INT;
            }
        if(node->token.type == TT_FLOAT)
            {
                return TT_FLOAT;
            }
        if(node->token.type == TT_STRING)
            {
                return TT_STRING;
            }
        if(node->token.type == TT_KEYWORD_Null)
            {
                return TT_KEYWORD_Null;
            }
        if(node->token.type == TT_IDENTIFIER)
            {
                token_type_t id_type = generator_get_id_type(node->token.lexeme, current_scope);
                if(id_type == TT_KEYWORD_STATIC && node->children == NULL)
                    {
                        return TT_INT;
                    }
                return id_type;
            }
        if(node->token.type == TT_PLUS || node->token.type == TT_MINUS || node->token.type == TT_MUL || node->token.type == TT_DIV)
            {
                token_type_t left_type = generator_get_type(node->children[0], current_scope);
                token_type_t right_type = generator_get_type(node->children[1], current_scope);
                if (left_type == TT_ERROR || right_type == TT_ERROR)
                    {
                        return TT_ERROR; 
                    }
                else if(node->token.type == TT_PLUS)
                    {
                        if(left_type == TT_STRING && right_type == TT_STRING)
                            {
                                return TT_STRING;
                            }
                    }
                if(left_type == TT_INT && right_type == TT_INT)
                    {
                        return TT_INT;
                    }
                else if(left_type == TT_FLOAT && right_type == TT_FLOAT)
                    {
                        return TT_FLOAT;
                    }
                else 
                    {
                        return TT_ERROR;
                    }
            }
        if(node->token.type == TT_ASSIGN)   
            {
                return generator_get_type(node->children[1], current_scope);
            }
        if(node->token.type == TT_GT || node->token.type == TT_LT || node->token.type == TT_EQ || node->token.type == TT_LE || node->token.type == TT_GE || node->token.type == TT_NEQ)
            {
                return TT_INT;
            }
        return TT_ERROR;
    }

//
//Here are all the print_ functions
//
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
