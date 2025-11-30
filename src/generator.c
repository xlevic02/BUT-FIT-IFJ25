//Implementace generatoru vysledneho kodu IFJcode25
//generator.c by Marek "xbalism00" Bališ on 11/11/25

#include "generator.h"
//#include "ast.h"
#include "ast.h"
#include "scanner.h"
//#include "symtable.h"
#include "error.h"
#include <stdio.h>

////
// This SHOULD be able to generate assembler code.
////

static int label_counter = 0;
static scope_stack stack;
static int var_counter = 0;

//I get the root node and symtable
int generate_code(ast_node_ptr root, bst_scope_ptr symtable)
    {
        if(root == NULL)    
            {
                return 0;
            }
        stack_init();
        printf(".IFJcode25\n");
        print_defvar("GF@%%tmp_op1");
        print_defvar("GF@%%tmp_op2");
        print_defvar("GF@%%tmp_res");
        print_defvar("GF@%%tmp_type");
        print_jump("$$main\n");
        //I assign roots number of children to int nu_of children
        int num_of_children = root->n_of_children;
        if(num_of_children > 0 && root->children == NULL)
            {
                return 0;
            }
        for(int i = 0; i < num_of_children; i++)    
            {
                if(root->children[i] == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, root, NULL);
                    }
                //Generate all the functions that are children of root
                if(root->children[i]->node_type == NT_FUNC_DECL)
                    {
                        if(root->children[i]->token.lexeme && strcmp(root->children[i]->token.lexeme, "main") != 0)
                            {
                                printf("Entering Generate Node\n"); // Debugging line
                                generate_node(root->children[i], &symtable);
                            }
                    }
            }
        //Start main
        print_label("$$main");
        printf("CREATEFRAME\n");
        printf("PUSHFRAME\n");
        //Finding main 
        ast_node_ptr main = NULL;
        for(int i = 0; i < num_of_children; i++)
            {
                if(root->children[i]->node_type == NT_FUNC_DECL && strcmp(root->children[i]->token.lexeme, "main") == 0)
                    {
                        main = root->children[i];
                        break;
                    }
            }
        //Generating main
        if(main)
            {
                stack_push(main);
                if(main->n_of_children > 1 && main->children != NULL && main->children[1] != NULL)
                    {
                        generate_node(main->children[1], &symtable);
                    }
                stack_pop();
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
        token_type_t type = node->token.type;
        ast_node_type_t ntype = node->node_type;
        char helper_buff[256];
        //Looking at the node's token type
        //If it's a function
        //printf("YES WE ARE HERE %s %d %d \n", node->token.lexeme, type, node->node_type); // Debugging line
        if(ntype == NT_FUNC_DECL)
                    {   
                        //Check if the node is main
                        if(strcmp(node->token.lexeme, "main") == 0)
                            {
                                return;
                            }
                        if(node->token.lexeme == NULL)
                            {
                                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                            }
                        //printf("Generating node %s\n", node->token.lexeme); // Debugging line
                        //fflush(stdout); // Ensure output is flushed
                        //We print the label of the function, push the alredy existing frame and go through the body
                        print_label(node->token.lexeme);
                        printf("PUSHFRAME\n");
                        print_defvar("LF@%%retval");
                        print_move("LF@%%retval", "nil@nil\n");
                        stack_push(node);
                        //Increasing the scope
                        //bst_increase_scope(current_scope);
                        //bst_generator_step_in(current_scope);
                        //If the node's parent is a function
                        //Get all of the functions parameters and define them
                        //fflush(stdout); // Ensure output is flushed
                        if(node->n_of_children && node->children != NULL)
                            {
                                ast_node_ptr parameters = node->children[0];
                                if(parameters && parameters->node_type == NT_PARAM)
                                    {
                                        for(int i = 0; i < parameters->n_of_children; i++)
                                            {
                                                char *parameter_name = parameters->children[i]->token.lexeme;
                                                char *unique = stack_register_var(parameter_name, parameters->children[i]);
                                                if(unique)
                                                    {
                                                        sprintf(helper_buff, "LF@%s", unique);
                                                        print_defvar(helper_buff);
                                                        char source_buff[32];
                                                        sprintf(source_buff, "LF@%%%d", i + 1);
                                                        print_move(helper_buff, source_buff);
                                                    }
                                            }
                                    }
                            }
                        if(node->n_of_children > 1)
                            {
                                generate_node(node->children[1], current_scope);
                            }
                        //fflush(stdout); // Doesnt get here
                        //bst_decrease_scope(current_scope);
                        stack_pop();
                        printf("POPFRAME\n");
                        printf("RETURN\n\n");
                        //fflush(stdout);
                    }
                //The body of a funcion, if/else or while
        else if(type == TT_LBRACE || ntype == NT_BLOCK)
                    {
                        bool is_function_block = (node->parent && node->parent->node_type == NT_FUNC_DECL);
                        //Go through the body
                        if(is_function_block == false)
                            {
                                stack_push(node);
                            }
                        if(node->children != NULL)
                            {
                                for(int i = 0; i < node->n_of_children; i++)
                                    {
                                        generate_node(node->children[i], current_scope);
                                    }
                            }
                        if(is_function_block == false)
                            {
                                stack_pop();
                            }
                    }
                //When it's an IF
        else if(type == TT_KEYWORD_IF)
                    {
                        //We get a label for the end and for else
                        char label_end[64];
                        char label_else[64];
                        get_unique_label(label_end, "if_end");
                        get_unique_label(label_else, "if_else");
                        if(node->n_of_children < 2 || node->children == NULL)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        //Then generate the conditions and check if they're true or false
                        generate_node(node->children[0], current_scope);
                        print_pops("GF@%%tmp_res");
                        print_pushs(VARIABLE, "%tmp_res", "GF");
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jumpifeqs(label_else);
                        print_pushs(VARIABLE, "%tmp_res", "GF");
                        print_pushs(LITERAL_NULL, "nil", NULL);
                        print_jumpifeqs(label_else);
                        generate_node(node->children[1], current_scope);
                        print_jump(label_end);
                        print_label(label_else);
                        if(node->n_of_children > 2 && node->children[2])
                            {
                                generate_node(node->children[2], current_scope);
                            }
                        print_label(label_end);

                    }
                //And now for while
        else if(type == TT_KEYWORD_WHILE)
                    {
                        //Get label for start and end
                        char w_start[64];
                        char w_end[64];
                        get_unique_label(w_start, "while_start");
                        get_unique_label(w_end, "while_end");
                        if(node->n_of_children < 2 || node->children == NULL)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        //We start the while, generate the conditions AND THEN check if ther're true
                        print_label(w_start);
                        generate_node(node->children[0], current_scope);
                        print_pops("GF@%%tmp_res");
                        print_pushs(VARIABLE, "%tmp_res", "GF");
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jumpifeqs(w_end);
                        print_pushs(VARIABLE, "%tmp_res", "GF");
                        print_pushs(LITERAL_NULL, "nil", NULL);
                        print_jumpifeqs(w_end);
                        generate_node(node->children[1], current_scope);
                        print_jump(w_start);
                        print_label(w_end);
                    }
                //Return
        else if(type == TT_KEYWORD_RETURN)
                    {
                        if(node->children != NULL && node->n_of_children > 0)
                            {
                                generate_node(node->children[0], current_scope);
                                print_pops("LF@%%retval");
                            }
                        else
                            {
                                print_move("LF@%%retval", "nil@nil");
                            }
                        printf("POPFRAME\n");
                        printf("RETURN\n");
                    }
                //Now for variables
        else if(type == TT_KEYWORD_VAR)
                    {
                        //We get the variables name
                        char* var_name = NULL;
                        //If the node has children, var_name will be the lexeme of children[0]
                        if(node->n_of_children > 0 && node->children != NULL && node->children[0] != NULL)
                            {
                                var_name = node->children[0]->token.lexeme;
                            }
                        //Else var_name will be the lexeme of the current token
                        else
                            {
                                var_name = node->token.lexeme;
                            }
                        //If the name is null, error
                        if(var_name == NULL)
                            {
                                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                            }
                        //Check if variable is global, if yes give it GF@ prefix, else give it a unique id and give it LF@ prefix
                        if(global_check(var_name))
                            {
                                sprintf(helper_buff, "GF@%s", var_name);
                                print_defvar(helper_buff);
                                print_move(helper_buff, "nil@nil");
                            }
                        else
                            {
                                char *unique = stack_register_var(var_name, node);
                                if(unique)
                                    {
                                        sprintf(helper_buff, "LF@%s", unique);
                                        print_defvar(helper_buff);
                                        print_move(helper_buff, "nil@nil");
                                    }
                            }
                    }
        //The first five ifs are basicaly what to push
        else if(type == TT_INT)
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
        else if(type == TT_KEYWORD_Null || type == TT_NULL)
            {
                print_pushs(LITERAL_NULL, "nil", NULL);
            }
        //If it's identifier and has children at the same time
        else if(type == TT_IDENTIFIER && node->n_of_children > 0)
            {
                //If token.lexeme is null, error
                if(node->token.lexeme == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                //If it's not null create func_name and give it token.lexeme
                char* func_name = node->token.lexeme;
                printf("CREATEFRAME\n");
                for(int i = 0; i < node->n_of_children; i++) 
                    {
                        generate_node(node->children[i], current_scope);
                        sprintf(helper_buff, "TF@%%%d", i+1);
                        print_defvar(helper_buff);
                        print_pops(helper_buff);
                    } 
                print_call(func_name);
                print_pushs(VARIABLE, "%retval", "TF");
            }
        //If it's just identifier
        else if(type == TT_IDENTIFIER)
            {
                //Same as before, if token.lexeme is null, error
                if(node->token.lexeme == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                char* var_name = node->token.lexeme;
                //Create buffer operand
                char operand[128];
                memset(operand, 0, 128);
                //Call resolve id to get the frame, id of var and var_name into operand
                stack_resolve_id(operand, var_name);
                if(operand[0] != '\0')
                    {
                        //Push operand 
                        printf("PUSHS %s\n", operand); 
                    }
            }
        //Here's assign
        else if(type == TT_ASSIGN)
            {
                //If it has less than two children or children is null, error
                if(node->n_of_children < 2 || node->children == NULL)
                    {
                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Else generate the second child, create target and give it the first child, then pop the result of the second child into the first, and then push the first
                generate_node(node->children[1], current_scope);
                ast_node_ptr target = node->children[0];
                char* var_name = target->token.lexeme;  
                //Create buffer operand                      
                char operand[128];
                memset(operand, 0, 128);
                //Call resolve id to get the frame, id of var and var_name into operand
                stack_resolve_id(operand, var_name);              
                if(operand[0] != '\0')
                    {
                        print_pops(operand);
                        printf("PUSHS %s\n", operand); 
                    }
            }
        //Then there's plus
        else if(type == TT_PLUS)
            {
                //Generate the children
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);                
                //Pop the results into helper operands
                print_pops("GF@%%tmp_op2");
                print_pops("GF@%%tmp_op1");
                //Push the first operand and check its type                
                print_pushs(VARIABLE, "%tmp_op1", "GF");
                print_types();
                print_pops("GF@%tmp_type");                
                char label_concat[64], label_add[64], label_end[64];
                get_unique_label(label_concat, "op_concat");
                get_unique_label(label_add, "op_add");
                get_unique_label(label_end, "op_end");
                //If the type is string we'll do concat
                printf("JUMPIFEQ %s GF@%%tmp_type string@string\n", label_concat);    
                //If it's int or float we'll do adds            
                print_pushs(VARIABLE, "%tmp_op1", "GF");
                print_pushs(VARIABLE, "%tmp_op2", "GF");
                print_arithmetic(ADD);
                print_pops("GF@%tmp_res");
                print_pushs(VARIABLE, "%tmp_res", "GF");
                print_jump(label_end);     
                print_label(label_concat);
                printf("CONCAT GF@%%tmp_res GF@%%tmp_op1 GF@%%tmp_op2\n");
                pprint_pushs(VARIABLE, "%tmp_res", "GF");              
                print_label(label_end);
            }
        //Minus and mul should be fine
        else if(type == TT_MINUS)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_arithmetic(SUB);
            }
        else if(type == TT_MUL)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_arithmetic(MUL);
            }
        else if(type == TT_DIV)
            {
                //Generate children
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                char op2_zero[64], label_idiv[64], label_div[64], label_end[64];
                get_unique_label(op2_zero, "op2_zero");
                //Pop the results into helper operands
                print_pops("GF@%%tmp_op2");
                print_pops("GF@%%tmp_op1");
                //Push the first operand and check its type                
                print_pushs(VARIABLE, "%tmp_op1", "GF");
                print_types(); 
                print_pops("GF@%tmp_type");                
                get_unique_label(label_idiv, "op_idiv");
                get_unique_label(label_div, "op_div");
                get_unique_label(label_end, "op_end");
                //If the operand is int, jump to idiv
                printf("JUMPIFEQ %s GF@%%tmp_type int@int\n", label_idiv);
                //If the operand is float, first check, if the second operand is not zero
                printf("JUMPIFEQ %s GF@%%tmp_op2\n float@0x0p+0\n", op2_zero);     
                //If its not zero, do the operation         
                print_pushs(VARIABLE, "%tmp_op1", "GF");
                print_pushs(VARIABLE, "%tmp_op2", "GF");
                print_arithmetic(DIV);
                print_pops("GF@%tmp_res");
                print_pushs(VARIABLE, "%tmp_res", "GF");
                print_jump(label_end);     
                print_label(label_idiv);
                //Here's the same as float, just for int
                printf("JUMPIFEQ %s GF@%%tmp_op2\n int@0\n", op2_zero); 
                print_pushs(VARIABLE, "%tmp_op1", "GF");
                print_pushs(VARIABLE, "%tmp_op2", "GF");
                print_arithmetic(IDIV);
                print_pops("GF@%tmp_res");
                print_pushs(VARIABLE, "%tmp_res", "GF"); 
                print_jump(label_end);
                print_label(op2_zero);
                print_exit("int@57");          
                print_label(label_end);
            }
        //All of relation types (I hope)
        else if(type == TT_GT)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_relation(GT);
            }
        else if(type == TT_LT)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_relation(LT);
            }
        else if(type == TT_EQ)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_relation(EQ);
            }
        else if(type == TT_LE)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_relation(GT);
                print_boolean(NOT);
            }
        else if(type == TT_GE)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_relation(LT);
                print_boolean(NOT);
            }
        else if(type == TT_NEQ)
            {
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                print_relation(EQ);
                print_boolean(NOT);
            }
        else if(type == TT_KEYWORD_IS)
            {
                //Generate the first child
                generate_node(node->children[0], current_scope);
                //Create type_node and give it the second child
                ast_node_ptr type_node = node->children[1];
                char label_true[64], label_false[64], label_end[64];
                get_unique_label(label_true, "true");
                get_unique_label(label_false, "false");
                get_unique_label(label_end, "end");
                //Pop the first child into helper operand, then check its type and pop it into helper type
                print_pops("GF@%%tmp_op1");
                print_pushs(VARIABLE, "%tmp_op1", "GF");
                print_types(); 
                print_pops("GF@%tmp_type");
                //If type_node is a number
                if(type_node->token.type == TT_KEYWORD_NUM)
                    {
                        //Jump to true if its type is int or float
                        printf("JUMPIFEQ %s GF@%%tmp_type string@int\n", label_true);
                        printf("JUMPIFEQ %s GF@%%tmp_type string@float\n", label_true);
                    } 
                //If type_node is string
                else if (type_node->token.type == TT_KEYWORD_STRING) 
                    {
                        //Jump to true if its type is string
                        printf("JUMPIFEQ %s GF@%%tmp_type string@string\n", label_true);
                    }
                //If type_node is null 
                else if (type_node->token.type == TT_KEYWORD_Null) 
                    {
                        //Jump to true if its type is null
                        printf("JUMPIFEQ %s GF@%%tmp_type string@nil\n", label_true);
                    }
                //If its false, we'll push false and jump to the end
                print_label(label_false);
                print_pushs(LITERAL_BOOL, "false", NULL);
                print_jump(label_end);
                //If its true, we'll push true
                print_label(label_true);
                print_pushs(LITERAL_BOOL, "true", NULL);
                print_label(label_end);
            }
        //And here's keyword_IFJ, which are builtin functions
        else if(type == TT_KEYWORD_IFJ)
            {
                //We basicaly get the tokens lexeme and see whats there so we know what to generate
                char *func_name = node->token.lexeme;
                if(func_name == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                if(strcmp(func_name, "write") == 0)
                    {
                        //WRITE
                        for(int i = 0; i < node->n_of_children; i++)
                            {
                                generate_node(node->children[i], current_scope);
                                print_pops("GF@%%tmp_op1");
                                print_write("GF@%%tmp_op1");
                            }
                        print_pushs(LITERAL_NULL, "nil", NULL);
                    }
                else if(strcmp(func_name, "read_str") == 0)
                    {
                        //READ_STR
                        print_read("GF@%%tmp_res", "string");
                        printf("PUSHS GF@%%tmp_res\n");
                    }
                else if(strcmp(func_name, "read_num") == 0)
                    {
                        //READ_NUM
                        print_read("GF@%%tmp_res", "int");
                        printf("PUSHS GF@%%tmp_res\n");
                    }
                else if(strcmp(func_name, "str") == 0)
                    {
                        //INT2STR, FLOAT2STR
                        if(node->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[0], current_scope);
                        print_pops("GF@%%tmp_op1");
                        print_pushs(VARIABLE, "%tmp_op1", "GF");
                        print_types(); 
                        print_pops("GF@%tmp_type");
                        char label_int[64], label_float[64], label_end[64];
                        get_unique_label(label_int, "str_is_int");
                        get_unique_label(label_float, "str_is_float");
                        get_unique_label(label_end, "str_end");
                        printf("JUMPIFEQ %s GF@%%tmp_type string@int\n", label_int);
                        printf("JUMPIFEQ %s GF@%%tmp_type string@float\n", label_float);
                        print_pushs(VARIABLE, "%tmp_op1", "GF");
                        print_jump(label_end);
                        print_label(label_int);
                        print_pushs(VARIABLE, "%tmp_op1", "GF");
                        print_conversion(INT2STR);
                        print_jump(label_end);
                        print_label(label_float);
                        print_pushs(VARIABLE, "%tmp_op1", "GF");
                        print_conversion(FLOAT2STR);                        
                        print_label(label_end);
                    }
                else if(strcmp(func_name, "floor") == 0)
                    {
                        //FLOAT2INT
                        if(node->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[0], current_scope);
                        print_conversion(FLOAT2INT);
                    }
                else if(strcmp(func_name, "length") == 0)
                    {
                        //STRLEN
                        if(node->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[0], current_scope);
                        print_pops("GF@%%tmp_op1");
                        print_strlen("GF@%%tmp_res", "GF@%%tmp_op1");
                        print_pushs(VARIABLE, "%tmp_res", "GF");
                    }
                else if(strcmp(func_name, "substring") == 0)
                    {
                                //Substring: Created vars to pop the arguments and the final result into
                                //Created a loop at the end to create the substring by putting the variable at index i into tmp_char and then using concat to add it to result_var
                                if(node->n_of_children < 3)
                                    {
                                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                                    }
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
                                generate_node(node->children[0], current_scope);
                                generate_node(node->children[1], current_scope);
                                generate_node(node->children[2], current_scope);
                                print_pops(j_var);
                                print_pops(i_var);
                                print_pops(str_var);
                                printf("PUSHS %s\n", i_var);
                                printf("PUSHS int@0\n");
                                print_relation(LT);
                                printf("PUSHS bool@true\n");
                                char label_err[32]; 
                                get_unique_label(label_err, "err58");
                                print_jumpifeqs(label_err);
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
                                print_label(label_err);
                                print_exit("int@58");
                                print_label(loop_end);
                                print_pushs(VARIABLE, result_var, "LF");

                    }
                else if(strcmp(func_name, "strcmp") == 0)
                    {
                        //Compare s1 with s2, and then pushes either -1, 1 or 0 depending on if s1 is shorter, longer or equal with s2
                        //Almost done, now I'll just have to create some additional variables to help, some jumps
                        // and the labels for those jumps
                        if(node->n_of_children < 2)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        char label_str1_lesser[32], label_str1_greater[32], label_strcmp_end[32];
                        get_unique_label(label_str1_lesser, "strcmp_lesser");
                        get_unique_label(label_str1_greater, "strcmp_greater");
                        get_unique_label(label_strcmp_end, "strcmp_end");
                        generate_node(node->children[0], current_scope);
                        generate_node(node->children[1], current_scope);
                        print_pops("GF@%%tmp_op2");
                        print_pops("GF@%%tmp_op1");
                        printf("LT GF@%%tmp_res GF@%%tmp_op1 GF@%%tmp_op2\n");
                        printf("JUMPIFEQ %s GF@%%tmp_res bool@true\n", label_str1_lesser);
                        printf("GT GF@%%tmp_res GF@%%tmp_op1 GF@%%tmp_op2\n");
                        printf("JUMPIFEQ %s GF@%%tmp_res bool@true\n", label_str1_greater);
                        printf("PUSHS int@0\n");
                        print_jump(label_strcmp_end);
                        print_label(label_str1_lesser);
                        printf("PUSHS int@-1\n");
                        print_jump(label_strcmp_end);
                        print_label(label_str1_greater);
                        printf("PUSHS int@1\n");
                        print_label(label_strcmp_end);
                    }
                else if(strcmp(func_name, "ord") == 0)
                    {
                        //STRI2INT
                        if(node->n_of_children < 2)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[0], current_scope);
                        generate_node(node->children[1], current_scope);
                        print_conversion(STRI2INT);
                    }
                else if(strcmp(func_name, "chr") == 0)
                    {
                        //INT2CHAR
                        if(node->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[0], current_scope);
                        print_conversion(INT2CHAR);
                    }
            }   
    }



//Checks if the variable is global
bool global_check(const char* var_name)
    {
        if(var_name && var_name[0] == '_' && var_name[1] == '_')
            {
                return true;
            }
        else
            {
                return false;
            }
    }

//Gets the ID
const char* get_variable_frame(const char* var_name)
    {
        if(global_check(var_name))
            {
                return "GF";
            }
        return "LF";
    }

//Gives a unique label
void get_unique_label(char* buffer, const char* prefix)
    {
        sprintf(buffer, "@@%s@%d", prefix, label_counter++);
    }

//Formats float for the final code
char* format_float_for_ifjcode(const char* lexeme)
    {
        if(lexeme == NULL)
            {
                return "0x0p+0";
            } 
        static char g_float_buffer[100];
        sprintf(g_float_buffer, "%a", strtod(lexeme, NULL));
        return g_float_buffer;
    }

//Formats string for final code
char* format_string_for_ifjcode(const char* lexeme)
    {
        if(lexeme == NULL)
            {
                return "";
            }
        static char g_string_buffer[10000];
        char* buf_ptr = g_string_buffer;
        int lexeme_len = strlen(lexeme);
        if(lexeme_len > 2000) 
            {
                lexeme_len = 2000;
            }
        for(int i = 0; i < lexeme_len; i++)
        {
            unsigned char c = lexeme[i];
            if(c <= 32 || c == 35 || c == 92) 
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
//Here I created a shadow stack to help me move through scopes
//

//Initializes shadow stack
void stack_init()
    {
        stack.top = -1;
    }

//Creates a new frame for the stack
void stack_push(ast_node_ptr node)
    {
        if(stack.top >= MAX_STACK_SIZE - 1)
            {
                if(node)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                else
                    {
                        exit(ERROR_GEN_INTERNAL);
                    }
            }
        //I move the stack top and initializes an empty array
        stack.top++;
        stack.arr[stack.top] = NULL;
    }

//Removes the scope from stack
void stack_pop() 
    {
        //If the top isn't empty
        if(stack.top != -1)
            {
                //I'll free all variables from the array
                var_node *current = stack.arr[stack.top];
                while(current != NULL)
                    {
                        var_node *next = current->next;
                        if(current->new_id)
                            {
                                free(current->new_id);
                            }
                        free(current);
                        current = next;
                    }
                //Then move the top back
                stack.arr[stack.top] = NULL;
                stack.top--;
            }
    }

//Adds variable to current scope and gives it a unique id
char* stack_register_var(char *var_name, ast_node_ptr node) 
    {
        if(var_name == NULL) 
            {
                return NULL;
            }
        if (stack.top == -1) 
            {
                return NULL;
            }
        //I create new var_node vae and allocate memory for it
        var_node *var = malloc(sizeof(var_node));
        if (var == NULL)
            {
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
            }
        //Then I place var_name into var->original_id
        var->original_id = var_name;
        //Then allocate space for new id
        var->new_id = malloc(strlen(var_name) + 32);
        if (var->new_id == NULL)
            {
                free(var);
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
            }
        //Then place into new_id var_name and var_counter
        sprintf(var->new_id, "%s$%d", var_name, var_counter++);
        
        //Then make the pointer next point at previous the stack top, then place var into stack top.
        var->next = stack.arr[stack.top];
        stack.arr[stack.top] = var;
        
        return var->new_id;
    }

//Helper function for getting the correct operand
void stack_resolve_id(char *buffer, char *var_name) 
    {
        if(buffer == NULL)
            {
                return;
            }
        if(var_name == NULL)
            {
                buffer[0] = '\0';
                return;
            }
        //I go from the top of the stack to the bottom
        for(int i = stack.top; i >= 0; i--)
            {
                //Create temporary var_node var and give it the current stack top.
                var_node *var = stack.arr[i];
                while(var != NULL)
                    {
                        //If the original_id is not null and the original_id is var_name
                        if(var->original_id != NULL && strcmp(var->original_id, var_name) == 0)
                            {
                                //Give it the LF@ prefix
                                sprintf(buffer, "LF@%s", var->new_id);
                                return;
                            }
                        //Move on to the next
                        var->next;
                    }
            }
        //If I don't find it in the stack, I give it the GF@ prefix
        sprintf(buffer, "GF@%s", var_name);
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
        if(value == NULL && type != LITERAL_NULL)
            {
                return;
            }
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
