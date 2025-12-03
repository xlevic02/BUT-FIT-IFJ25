//Implementace generatoru vysledneho kodu IFJcode25
//generator.c by Marek "xbalism00" Bališ on 11/11/25

#include "generator.h"

static int label_counter = 0;
static scope_stack stack;
static int var_counter = 0;
static ast_node_ptr root_node = NULL; 
static id_list_item *saved_ids = NULL;

//Get the root node and symtable
int generate_code(ast_node_ptr root, bst_scope_ptr symtable)
    {
        if(root == NULL)    
            {
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, root, NULL);
            }
        root_node = root;
        stack_init();
        printf(".IFJcode25\n");
        //Here I define some helper global vars
        print_defvar("GF@tmp_op1");
        print_defvar("GF@tmp_op2");
        print_defvar("GF@tmp_res");
        print_defvar("GF@tmp_type");
        //Generate all the global variables in symtable
        if (symtable != NULL && symtable->tree != NULL)
            {
                generate_globals(symtable->tree);
            }
        printf("CREATEFRAME\n");
        print_call("$$main");
        print_exit("int@0");
        //I assign roots number of children to int nu_of children
        int num_of_children = root->n_of_children;
        if(num_of_children > 0 && root->children == NULL)
            {
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, root, NULL);
            }
        for(int i = 0; i < num_of_children; i++)    
            {
                if(root->children[i] == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, root, NULL);
                    }
                //Generate all the functions that are children of root
                ast_node_type_t ntype = root->children[i]->node_type;
                if(ntype == NT_FUNC_DECL || ntype == NT_GETTER || ntype == NT_SETTER)
                    {
                        if(root->children[i]->token.lexeme && strcmp(root->children[i]->token.lexeme, "main") != 0)
                            {
                                generate_node(root->children[i], &symtable);
                            }
                    }
            }
        //Start main
        print_label("$$main");
        printf("CREATEFRAME\n");
        printf("PUSHFRAME\n");
        print_defvar("LF@retval");
        print_move("LF@retval", "nil@nil");
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
                cleanup_ids();
                //Define all the variables that are in main body
                if(main->n_of_children > 1) 
                    {
                        scan_variables(main->children[1]);
                    }
                stack_pop();
                stack_push(main);
                if(main->n_of_children > 1 && main->children != NULL && main->children[1] != NULL)
                    {
                        generate_node(main->children[1], &symtable);
                    }
                cleanup_ids();
                stack_pop();
            }
        printf("POPFRAME\n");
        printf("RETURN\n");
        return 0;
    }

//Generate the nodes themselves
void generate_node(ast_node_ptr node, bst_scope_ptr *current_scope)
    {
        if(node == NULL)
            {
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
            }
        //Get the tokens and the nodes type 
        token_type_t type = node->token.type;
        ast_node_type_t ntype = node->node_type;
        char helper_buff[256];
        //If it's a function
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
                //We print the label of the function, push the alredy existing frame and go through the body
                char label_buffer[256];
                sprintf(label_buffer, "%s$%d", node->token.lexeme, node->children[0]->n_of_children);
                print_label(label_buffer);
                printf("PUSHFRAME\n");
                printf("CREATEFRAME\n");
                print_defvar("LF@retval");
                print_move("LF@retval", "nil@nil");
                //Define all the variables that are in functions body
                stack_push(node);
                cleanup_ids();
                if(node->n_of_children > 1) 
                    {
                        scan_variables(node->children[1]);
                    }
                stack_pop();
                stack_push(node);
                if(node->n_of_children && node->children != NULL)
                    {
                        //Get the parameters
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
                                                sprintf(source_buff, "LF@par%d", i + 1);
                                                print_move(helper_buff, source_buff);
                                            }
                                    }
                            }
                    }
                //Do the body
                if(node->n_of_children > 1)
                    {
                        generate_node(node->children[1], current_scope);
                    }
                //Pop current scope
                cleanup_ids();
                stack_pop();
                printf("POPFRAME\n");
                printf("RETURN\n");
            }
        //If its a getter
        else if(ntype == NT_GETTER)
            {
                if(node->token.lexeme == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                if(node->n_of_children < 1)
                    {
                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Give it getter label
                char label_buffer[256];
                sprintf(label_buffer, "getter$%s", node->token.lexeme);
                print_label(label_buffer);
                printf("PUSHFRAME\n");
                printf("CREATEFRAME\n");
                print_defvar("LF@retval");
                print_move("LF@retval", "nil@nil");
                //Define all variables in its body
                stack_push(node);
                cleanup_ids();
                scan_variables(node->children[0]);
                stack_pop();
                //Push stack and do the body
                stack_push(node);
                generate_node(node->children[0], current_scope);
                //Pop stack
                stack_pop();
                cleanup_ids();
                printf("POPFRAME\n");
                printf("RETURN\n");
            }
        //If its a setter
        else if(ntype == NT_SETTER)
            {
                if(node->token.lexeme == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                if(node->n_of_children < 2)
                    {
                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Give it a setter label
                char label_buffer[256];
                sprintf(label_buffer, "setter$%s", node->token.lexeme);
                print_label(label_buffer);
                printf("PUSHFRAME\n");
                printf("CREATEFRAME\n");
                print_defvar("LF@retval");
                print_move("LF@retval", "nil@nil");
                //Define all variables in its body
                stack_push(node);
                cleanup_ids();
                scan_variables(node->children[1]); 
                stack_pop();
                //Push stack
                stack_push(node);
                //Get and define parameter
                if (node->children[0] && node->children[0]->n_of_children > 0) 
                    {
                        ast_node_ptr parameter_node = node->children[0]->children[0];
                        char *param_name = parameter_node->token.lexeme;
                        char *unique = stack_register_var(param_name, parameter_node);
                        if(unique)
                            {
                                sprintf(helper_buff, "LF@%s", unique);
                                print_defvar(helper_buff);
                                print_move(helper_buff, "LF@par1");
                            }
                    }
                //Do the body
                generate_node(node->children[1], current_scope);
                //Pop stack
                stack_pop();
                cleanup_ids();
                printf("POPFRAME\n");
                printf("RETURN\n");
            }
        //The body of a funcion, getter, setter, if/else or while
        else if(type == TT_LBRACE || ntype == NT_BLOCK || ntype == NT_IF_BODY || ntype == NT_ELSE_BODY || ntype == NT_WHILE_BODY)
                    {
                        bool is_function_block = (node->parent && (node->parent->node_type == NT_FUNC_DECL || node->parent->node_type == NT_GETTER || node->parent->node_type == NT_SETTER));
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
                for(int i = 0; i < node->children[0]->n_of_children; i++) 
                    {
                        generate_node(node->children[0]->children[i], current_scope);
                        sprintf(helper_buff, "TF@par%d", i+1);
                        print_defvar(helper_buff);
                        print_pops(helper_buff);
                    } 
                char label_buffer[256];
                sprintf(label_buffer, "%s$%d", func_name, node->children[0]->n_of_children);
                print_call(label_buffer);
                print_pushs(VARIABLE, "retval", "TF");
            }
        else if(ntype == NT_ID)
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
                //Check if it's global and check if it's a getter
                if (strncmp(operand, "GF@", 3) == 0 && get_function_type(var_name, 0) == 2) 
                    {
                        printf("CREATEFRAME\n");
                        char getter_label[256];
                        sprintf(getter_label, "getter$%s", var_name);
                        print_call(getter_label);
                        print_pushs(VARIABLE, "retval", "TF");
                    } 
                else if(operand[0] != '\0')
                    {
                        printf("PUSHS %s\n", operand); 
                    }
                else
                    {
                        printf("PUSHS %s\n", var_name);
                    }
            }
        //Now for variables
        else if(type == TT_KEYWORD_VAR || ntype == NT_VAR_DEF)
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
                //Check if variable is global, if yes give it GF@ prefix, else find its id and give it LF@ prefix
                if(global_check(var_name))
                    {
                        sprintf(helper_buff, "GF@%s", var_name);
                        print_move(helper_buff, "nil@nil");
                    }
                else
                    {
                        //Get its id
                        char *existing_id = find_var_id(node);
                        if (existing_id) 
                            {
                                stack_push_existing(var_name, existing_id, node);
                                sprintf(helper_buff, "LF@%s", existing_id);
                                print_move(helper_buff, "nil@nil");
                            } 
                        else 
                            {
                                //Fallback
                                char *unique = stack_register_var(var_name, node);
                                if(unique)
                                    {
                                        sprintf(helper_buff, "LF@%s", unique);
                                        print_defvar(helper_buff);
                                        print_move(helper_buff, "nil@nil");
                                    }
                            }
                    }
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
                //Check if it's global and check if it's a getter
                if (strncmp(operand, "GF@", 3) == 0 && get_function_type(var_name, 0) == 2) 
                    {
                        printf("CREATEFRAME\n");
                        char getter_label[256];
                        sprintf(getter_label, "getter$%s", var_name);
                        print_call(getter_label);
                        print_pushs(VARIABLE, "retval", "TF");
                    } 
                else if(operand[0] != '\0')
                    {
                        printf("PUSHS %s\n", operand); 
                    }
                else
                    {
                        printf("PUSHS %s\n", var_name);
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
                //Then check if operand is global and if it is check if it's a setter           
                if (strncmp(operand, "GF@", 3) == 0 && get_function_type(var_name, 1) == 3) 
                    {
                        print_pops("GF@tmp_res");
                        printf("CREATEFRAME\n");
                        print_defvar("TF@par1");
                        print_move("TF@par1", "GF@tmp_res"); 
                        char label_buffer[256];
                        sprintf(label_buffer, "setter$%s", var_name);
                        print_call(label_buffer);
                        print_pushs(VARIABLE, "retval", "TF");
                    }
                else
                    {
                        print_pops(operand);
                    } 
            }
        //When it's an IF
        else if(type == TT_KEYWORD_IF)
            {
                //We get a label for the end and for else
                char label_end[64];
                char label_else[64];
                char label_true[64];
                get_unique_label(label_end, "if_end");
                get_unique_label(label_else, "if_else");
                get_unique_label(label_true, "if_true");
                if(node->n_of_children < 2 || node->children == NULL)
                    {
                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Then generate the conditions and check if they're true or false
                generate_node(node->children[0], current_scope);
                print_pops("GF@tmp_res");
                print_pushs(VARIABLE, "tmp_res", "GF");
                print_pushs(LITERAL_NULL, "nil", NULL);
                print_jumpifeqs(label_else);
                print_pushs(VARIABLE, "tmp_res", "GF");
                print_types();
                print_pushs(LITERAL_STRING, "bool", NULL);
                print_jumpifneqs(label_true);
                print_pushs(VARIABLE, "tmp_res", "GF");
                print_pushs(LITERAL_BOOL, "false", NULL);
                print_jumpifeqs(label_else);
                print_label(label_true);
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
                char w_true[64];
                get_unique_label(w_start, "while_start");
                get_unique_label(w_end, "while_end");
                get_unique_label(w_true, "while_true");
                if(node->n_of_children < 2 || node->children == NULL)
                   {
                       ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Start the while, generate the conditions AND THEN check if ther're true
                print_label(w_start);
                generate_node(node->children[0], current_scope);
                print_pops("GF@tmp_res");
                print_pushs(VARIABLE, "tmp_res", "GF");
                print_pushs(LITERAL_NULL, "nil", NULL);
                print_jumpifeqs(w_end);
                print_pushs(VARIABLE, "tmp_res", "GF");
                print_types();
                print_pushs(LITERAL_STRING, "bool", NULL);
                print_jumpifneqs(w_true);
                print_pushs(VARIABLE, "tmp_res", "GF");
                print_pushs(LITERAL_BOOL, "false", NULL);
                print_jumpifeqs(w_end);
                print_label(w_true);
                //If they are, do the body
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
                        print_pops("LF@retval");
                            }
                else
                    {
                        print_move("LF@retval", "nil@nil");
                    }
                printf("POPFRAME\n");
                printf("RETURN\n");
            }
        //The first four ifs are basicaly what to push
        else if(type == TT_INT)
            {
                print_pushs(LITERAL_INT, node->token.lexeme, NULL);
            }
        else if(type == TT_FLOAT)
            {
                print_pushs(LITERAL_FLOAT, format_float_for_ifjcode(node->token.lexeme), NULL);
            }
        else if(type == TT_STRING || ntype == NT_DATATYPE)
            {
                print_pushs(LITERAL_STRING, format_string_for_ifjcode(node->token.lexeme), NULL);
            }
        else if(type == TT_KEYWORD_Null || type == TT_NULL)
            {
                print_pushs(LITERAL_NULL, "nil", NULL);
            }
        //If its an arithmetic expression
        else if(ntype == NT_AR_EXPR)
            {
                if(node->n_of_children < 2)
                    {
                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Generate children
                generate_node(node->children[0], current_scope);
                generate_node(node->children[1], current_scope);
                //Pop the results into helper operands
                print_pops("GF@tmp_op2");
                print_pops("GF@tmp_op1");
                //Then if op1 or op2 is an int, change them to float
                char label_op1_float[64], label_op2_float[64];
                get_unique_label(label_op1_float, "op1_int");
                get_unique_label(label_op2_float, "op1_int");
                print_pushs(VARIABLE, "tmp_op1", "GF");
                print_types();
                print_pops("GF@tmp_type");
                printf("JUMPIFNEQ %s GF@tmp_type string@int\n", label_op1_float);
                print_pushs(VARIABLE, "tmp_op1", "GF");
                print_conversion(INT2FLOAT);
                print_pops("GF@tmp_op1");
                print_label(label_op1_float);
                print_pushs(VARIABLE, "tmp_op2", "GF");
                print_types();
                print_pops("GF@tmp_type");
                printf("JUMPIFNEQ %s GF@tmp_type string@int\n", label_op2_float);
                print_pushs(VARIABLE, "tmp_op2", "GF");
                print_conversion(INT2FLOAT);
                print_pops("GF@tmp_op2");
                print_label(label_op2_float);
                //All arithmetic expressions
                if(type == TT_PLUS)
                    {  
                        char label_concat[64], label_add[64], label_end[64];
                        get_unique_label(label_concat, "op_concat");
                        get_unique_label(label_add, "op_add");
                        get_unique_label(label_end, "op_end");             
                        //Push the first operand and check its type                
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types();
                        print_pops("GF@tmp_type");                
                        //If the type is string we'll do concat
                        printf("JUMPIFEQ %s GF@tmp_type string@string\n", label_concat);    
                        //If it's int or float we'll do adds            
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_arithmetic(ADD);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_jump(label_end);     
                        print_label(label_concat);
                        print_concat("GF@tmp_res", "GF@tmp_op1", "GF@tmp_op2");
                        print_pushs(VARIABLE, "tmp_res", "GF");              
                        print_label(label_end);
                    }
                else if(type == TT_MINUS)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_arithmetic(SUB);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_MUL)
                    {
                        char label_string_op1[64], label_string_op2[64], label_mul[64], label_end[64], label_swap_op1_op2[64], label_start[64], label_loop_start[64], label_loop_end[64];
                        get_unique_label(label_string_op1, "op_1_string");
                        get_unique_label(label_string_op2, "op_1_string");
                        get_unique_label(label_mul, "op_mul");
                        get_unique_label(label_swap_op1_op2, "swap");
                        get_unique_label(label_start, "start");
                        get_unique_label(label_loop_start, "loop_start");
                        get_unique_label(label_loop_end, "loop_end");
                        get_unique_label(label_end, "op_end");
                        //Check if op1 is a string              
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types();
                        print_pops("GF@tmp_type");                
                        printf("JUMPIFEQ %s GF@tmp_type string@string\n", label_string_op1);
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_types();
                        print_pops("GF@tmp_type");
                        //Check if op2 is a string
                        printf("JUMPIFEQ %s GF@tmp_type string@string\n", label_string_op2);
                        //If neither is we multiply them               
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_jump(label_mul); 
                        //If op1 is a string  
                        print_label(label_string_op1);
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_types();
                        print_pops("GF@tmp_type");
                        //Check if op2 is a number
                        printf("JUMPIFEQ %s GF@tmp_type string@float\n", label_start);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_jump(label_mul);
                        //If op2 is a string
                        print_label(label_string_op2);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types();
                        print_pops("GF@tmp_type");
                        //Check if op1 is a number
                        printf("JUMPIFEQ %s GF@tmp_type string@float\n", label_swap_op1_op2);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_jump(label_mul);
                        //If op2 is a string and op1 a number, switch them using tmp_res
                        print_label(label_swap_op1_op2);
                        print_move("GF@tmp_res", "GF@tmp_op1");
                        print_move("GF@tmp_op1", "GF@tmp_op2");
                        print_move("GF@tmp_op2", "GF@tmp_res");
                        print_label(label_start);
                        //Then change op2 to int
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_conversion(FLOAT2INT);
                        print_pops("GF@tmp_op2");
                        //Move empty string to res
                        print_move("GF@tmp_res", "string@");
                        //Start the loop
                        print_label(label_loop_start);
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_relation(GT);
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jumpifeqs(label_loop_end);
                        print_concat("GF@tmp_res", "GF@tmp_res", "GF@tmp_op1");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_pushs(LITERAL_INT, "1", NULL);
                        print_arithmetic(SUB);
                        print_pops("GF@tmp_op2");
                        print_jump(label_loop_start);
                        print_label(label_loop_end);
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_jump(label_end);
                        print_label(label_mul);            
                        print_arithmetic(MUL);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF"); 
                        print_label(label_end);
                    }
                else if(type == TT_DIV)
                    {
                        char op2_zero[64], label_idiv[64], label_div[64], label_end[64];
                        get_unique_label(op2_zero, "op2_zero");
                        get_unique_label(label_idiv, "op_idiv");
                        get_unique_label(label_div, "op_div");
                        get_unique_label(label_end, "op_end");
                        //Push the first operand and check its type                
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types(); 
                        print_pops("GF@tmp_type");                
                        //If the operand is int, jump to idiv
                        printf("JUMPIFEQ %s GF@tmp_type string@int\n", label_idiv);
                        //If the operand is float, first check, if the second operand is not zero
                        printf("JUMPIFEQ %s GF@tmp_op2 float@0x0p+0\n", op2_zero);     
                        //If its not zero, do the operation         
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_arithmetic(DIV);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_jump(label_end);     
                        print_label(label_idiv);
                        //Here's the same as float, just for int
                        printf("JUMPIFEQ %s GF@tmp_op2 int@0\n", op2_zero); 
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_arithmetic(IDIV);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF"); 
                        print_jump(label_end);
                        print_label(op2_zero);
                        print_exit("int@57");          
                        print_label(label_end);
                    }
            }
        //All boolean expressions
        else if(ntype == NT_BOOL_EXPR)
            {
                if(node->n_of_children < 1)
                    {
                        ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                    }
                //Its the same as NT_AR_EXPR
                //Generate children
                generate_node(node->children[0], current_scope);
                if(node->n_of_children > 1)
                    {
                        generate_node(node->children[1], current_scope);
                    }
                //Pop the results into helper operands
                print_pops("GF@tmp_op2");
                print_pops("GF@tmp_op1");
                char label_op1_float[64], label_op2_float[64];
                get_unique_label(label_op1_float, "op1_int");
                get_unique_label(label_op2_float, "op1_int");
                print_pushs(VARIABLE, "tmp_op1", "GF");
                print_types();
                print_pops("GF@tmp_type");
                printf("JUMPIFNEQ %s GF@tmp_type string@int\n", label_op1_float);
                print_pushs(VARIABLE, "tmp_op1", "GF");
                print_conversion(INT2FLOAT);
                print_pops("GF@tmp_op1");
                print_label(label_op1_float);
                print_pushs(VARIABLE, "tmp_op2", "GF");
                print_types();
                print_pops("GF@tmp_type");
                printf("JUMPIFNEQ %s GF@tmp_type string@int\n", label_op2_float);
                print_pushs(VARIABLE, "tmp_op2", "GF");
                print_conversion(INT2FLOAT);
                print_pops("GF@tmp_op2");
                print_label(label_op2_float);
                //All arithmetic expressions
                if(type == TT_GT)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(GT);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_LT)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_EQ)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(EQ);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_LE)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(GT);
                        print_boolean(NOT);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_GE)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(LT);
                        print_boolean(NOT);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_NEQ)
                    {
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(EQ);
                        print_boolean(NOT);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(type == TT_KEYWORD_IS)
                    {
                        char label_true[64], label_false[64], label_end[64], label_is_num[64], label_is_string[64], label_is_nil[64];
                        get_unique_label(label_true, "true");
                        get_unique_label(label_false, "false");
                        get_unique_label(label_end, "end");
                        get_unique_label(label_is_num, "is_num");
                        get_unique_label(label_is_string, "is_string");
                        get_unique_label(label_is_nil, "is_nil");
                        //Pop the first child into helper operand, then check its type and pop it into helper type
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types(); 
                        print_pops("GF@tmp_type");
                        //If type_node is a number
                        printf("JUMPIFEQ %s GF@tmp_op2 string@Num\n", label_is_num);
                        printf("JUMPIFEQ %s GF@tmp_op2 string@String\n", label_is_string);
                        printf("JUMPIFEQ %s GF@tmp_op2 string@Null\n", label_is_nil);
                        //Jump to true if its type is int or float
                        print_label(label_is_num);
                        printf("JUMPIFEQ %s GF@tmp_type string@float\n", label_true);
                        print_jump(label_false);
                        //If type_node is string
                        //Jump to true if its type is string
                        print_label(label_is_string);
                        printf("JUMPIFEQ %s GF@tmp_type string@string\n", label_true);
                        print_jump(label_false);
                        //If type_node is null 
                        //Jump to true if its type is null
                        print_label(label_is_nil);
                        printf("JUMPIFEQ %s GF@tmp_type string@nil\n", label_true);
                        //If its false, we'll push false and jump to the end
                        print_label(label_false);
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jump(label_end);
                        //If its true, we'll push true
                        print_label(label_true);
                        print_pushs(LITERAL_BOOL, "true", NULL);
                        print_label(label_end);
                    }
            }
        //And here's keyword_IFJ, which are builtin functions
        else if(type == TT_KEYWORD_IFJ)
            {
                //We basicaly get the tokens lexeme and see whats there so we know what to generate
                char *builtin_type = node->children[0]->token.lexeme;
                if(builtin_type == NULL)
                    {
                        ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);
                    }
                if(strcmp(builtin_type, "write") == 0)
                    {
                        //WRITE
                        for(int i = 0; i < node->children[1]->n_of_children; i++)
                            {
                                generate_node(node->children[1]->children[i], current_scope);
                                print_pops("GF@tmp_op1");
                                print_write("GF@tmp_op1");
                            }
                        print_pushs(LITERAL_NULL, "nil", NULL);
                    }
                else if(strcmp(builtin_type, "read_str") == 0)
                    {
                        //READ_STR
                        print_read("GF@tmp_res", "string");
                        printf("PUSHS GF@tmp_res\n");
                    }
                else if(strcmp(builtin_type, "read_num") == 0)
                    {
                        //READ_NUM
                        print_read("GF@tmp_res", "float");
                        printf("PUSHS GF@tmp_res\n");
                    }
                else if(strcmp(builtin_type, "str") == 0)
                    {
                        //INT2STR, FLOAT2STR
                        if(node->children[1]->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[1]->children[0], current_scope);
                        //Check if its a float or int
                        print_pops("GF@tmp_op1");
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types(); 
                        print_pops("GF@tmp_type");
                        char label_int[64], label_float[64], label_end[64];
                        get_unique_label(label_int, "str_is_int");
                        get_unique_label(label_float, "str_is_float");
                        get_unique_label(label_end, "str_end");
                        printf("JUMPIFEQ %s GF@tmp_type string@int\n", label_int);
                        printf("JUMPIFEQ %s GF@tmp_type string@float\n", label_float);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_jump(label_end);
                        print_label(label_int);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_conversion(INT2STR);
                        print_jump(label_end);
                        print_label(label_float);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_conversion(FLOAT2STR);                        
                        print_label(label_end);
                    }
                else if(strcmp(builtin_type, "floor") == 0)
                    {
                        //FLOAT2INT
                        if(node->children[1]->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[1]->children[0], current_scope);
                        print_conversion(FLOAT2INT);
                    }
                else if(strcmp(builtin_type, "length") == 0)
                    {
                        //STRLEN
                        if(node->children[1]->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[1]->children[0], current_scope);
                        print_pops("GF@tmp_op1");
                        print_strlen("GF@tmp_res", "GF@tmp_op1");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                    }
                else if(strcmp(builtin_type, "substring") == 0)
                    {
                        //Substring: Created vars to pop the arguments and the final result into
                        //Created a loop at the end to create the substring by putting the variable at index i into tmp_char and then using concat to add it to result_var
                        if(node->children[1]->n_of_children < 3)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        char str_var[32], i_var[32], j_var[32], tmp_char[32];
                        char label_loop_start[64], label_loop_end[64], label_error[64], label_i_is_int[64], label_j_is_int[64];
                        sprintf(str_var, "LF@substr%d", label_counter);
                        sprintf(i_var, "LF@substr_i%d", label_counter);
                        sprintf(j_var, "LF@substr_j%d", label_counter);
                        sprintf(tmp_char, "LF@substr%d", label_counter++);
                        get_unique_label(label_loop_start, "substr_loop_start");
                        get_unique_label(label_loop_end, "substr_loop_end");
                        get_unique_label(label_i_is_int, "i_is_int");
                        get_unique_label(label_j_is_int, "j_is_int");
                        get_unique_label(label_error, "op_err");
                        print_defvar(str_var);
                        print_defvar(i_var);
                        print_defvar(j_var);
                        print_defvar(tmp_char);
                        generate_node(node->children[1]->children[0], current_scope);
                        generate_node(node->children[1]->children[1], current_scope);
                        generate_node(node->children[1]->children[2], current_scope);
                        print_pops(j_var);
                        print_pops(i_var);
                        print_pops(str_var);
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_types();
                        print_pops("GF@tmp_type");
                        printf("JUMPIFEQ %s GF@tmp_type string@int\n", label_i_is_int);
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_conversion(FLOAT2INT);
                        print_pops(i_var);
                        print_label(label_i_is_int);
                        print_pushs(VARIABLE, j_var + 3, "LF");
                        print_types();
                        print_pops("GF@tmp_type");
                        printf("JUMPIFEQ %s GF@tmp_type string@int\n", label_j_is_int);
                        print_pushs(VARIABLE, j_var + 3, "LF");
                        print_conversion(FLOAT2INT);
                        print_pops(j_var);
                        print_label(label_j_is_int);
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_error);
                        print_pushs(VARIABLE, j_var + 3, "LF");
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_error);
                        print_pushs(VARIABLE, j_var + 3, "LF");
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_error);
                        print_strlen("GF@tmp_res", str_var);
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_relation(LT);
                        print_boolean(NOT);
                        print_pops("GF@tmp_type");
                        printf("JUMPIFEQ %s GF@tmp_type bool@true\n", label_error);
                        print_pushs(VARIABLE, j_var + 3, "LF");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_relation(GT);
                        print_pops("GF@tmp_type");
                        printf("JUMPIFEQ %s GF@tmp_type bool@true\n", label_error);
                        print_move("GF@tmp_res", "string@");
                        print_label(label_loop_start);
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_pushs(VARIABLE, j_var + 3, "LF");
                        print_relation(LT);
                        print_pushs(LITERAL_BOOL, "false", NULL);
                        print_jumpifeqs(label_loop_end);
                        print_getchar(tmp_char, str_var, i_var);
                        print_concat("GF@tmp_res", "GF@tmp_res", tmp_char);
                        print_pushs(VARIABLE, i_var + 3, "LF");
                        print_pushs(LITERAL_INT, "1", NULL);
                        print_arithmetic(ADD);
                        print_pops(i_var);
                        print_jump(label_loop_start);
                        print_label(label_error);
                        print_pushs(LITERAL_NULL, "nil", NULL);
                        print_pops("GF@tmp_res");
                        print_label(label_loop_end);
                        print_pushs(VARIABLE, "tmp_res", "GF");

                    }
                else if(strcmp(builtin_type, "strcmp") == 0)
                    {
                        //Compare s1 with s2, and then pushes either -1, 1 or 0 depending on if s1 is shorter, longer or equal with s2
                        //Almost done, now I'll just have to create some additional variables to help, some jumps
                        // and the labels for those jumps
                        if(node->children[1]->n_of_children < 2)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        char label_str1_lesser[64], label_str1_greater[64], label_strcmp_end[64];
                        get_unique_label(label_str1_lesser, "strcmp_lesser");
                        get_unique_label(label_str1_greater, "strcmp_greater");
                        get_unique_label(label_strcmp_end, "strcmp_end");
                        generate_node(node->children[1]->children[0], current_scope);
                        generate_node(node->children[1]->children[1], current_scope);
                        print_pops("GF@tmp_op2");
                        print_pops("GF@tmp_op1");
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_str1_lesser);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(GT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_str1_greater);
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_jump(label_strcmp_end);
                        print_label(label_str1_lesser);
                        print_pushs(LITERAL_INT, "-1", NULL);
                        print_jump(label_strcmp_end);
                        print_label(label_str1_greater);
                        print_pushs(LITERAL_INT, "1", NULL);
                        print_label(label_strcmp_end);
                    }
                else if(strcmp(builtin_type, "ord") == 0)
                    {
                        //STRI2INT
                        if(node->children[1]->n_of_children < 2)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        char label_error[64], label_end[64], label_op2_int[64];
                        get_unique_label(label_error, "op_error");
                        get_unique_label(label_end, "op_end");
                         get_unique_label(label_op2_int, "op2_int");
                        generate_node(node->children[1]->children[0], current_scope);
                        generate_node(node->children[1]->children[1], current_scope);
                        print_pops("GF@tmp_op2");
                        print_pops("GF@tmp_op1");
                        //Check if op2 is int
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_types();
                        print_pops("GF@tmp_type");
                        printf("JUMPIFEQ %s GF@tmp_type string@int\n", label_op2_int);
                        //If not, change it to int
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_conversion(FLOAT2INT);
                        print_pops("GF@tmp_op2");
                        print_label(label_op2_int);
                        //Get the strings length
                        print_strlen("GF@tmp_res", "GF@tmp_op1");
                        //Compare them, if res is smaller than op2
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        //If its true, jump to error
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_error);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(VARIABLE, "tmp_op2", "GF");
                        print_conversion(STRI2INT);
                        print_pops("GF@tmp_res");
                        print_pushs(VARIABLE, "tmp_res", "GF");
                        print_jump(label_end);
                        print_label(label_error);
                        print_exit("int@58");
                        print_label(label_end);
                    }
                else if(strcmp(builtin_type, "chr") == 0)
                    {
                        //INT2CHAR
                        if(node->children[1]->n_of_children < 1)
                            {
                                ast_error(ERROR_ACCESS_NONEXISTENT_VAR, MSG_ACCESS_NONEXISTENT_VAR, node, NULL);
                            }
                        generate_node(node->children[1]->children[0], current_scope);
                        char label_error[64], label_end[64], label_op1_int[64];
                        get_unique_label(label_error, "op_error");
                        get_unique_label(label_end, "op_end");
                        get_unique_label(label_op1_int, "op1_int");
                        print_pops("GF@tmp_op1");
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_types();
                        print_pops("GF@tmp_type");
                        //Check if op1 is int
                        printf("JUMPIFEQ %s GF@tmp_type string@int\n", label_op1_int);
                        //If not, change it to int
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_conversion(FLOAT2INT);
                        print_pops("GF@tmp_op1");
                        print_label(label_op1_int);
                        //If it's smaller than zero, error
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(LITERAL_INT, "0", NULL);
                        print_relation(LT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_error);
                        //If its bigger than 255, error
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_pushs(LITERAL_INT, "255", NULL);
                        print_relation(GT);
                        print_pops("GF@tmp_res");
                        printf("JUMPIFEQ %s GF@tmp_res bool@true\n", label_error);
                        print_pushs(VARIABLE, "tmp_op1", "GF");
                        print_conversion(INT2CHAR);
                        print_jump(label_end);
                        print_label(label_error);
                        print_exit("int@58");
                        print_label(label_end);
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

//Function to search symtable for global vars and define them at the start
void generate_globals(bst_node_ptr symtable)
    {
        if (symtable == NULL)
            {
                return;
            }
        generate_globals(symtable->left);
        char *name = symtable->content.name;
        if (global_check(name))
            {
                printf("DEFVAR GF@%s\n", name);
                printf("MOVE GF@%s nil@nil\n", name);
            }
        generate_globals(symtable->right);
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
        sprintf(buffer, "%s$%d", prefix, label_counter++);
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

//Helper function to help differentiate between a regular function, a getter and a setter
int get_function_type(char* func_name, int parameter_count)
    {
        if(root_node == NULL)
            {
                // Ak nemame root, nemozeme uvolnit AST, posielame NULL
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, NULL, NULL);
            }
        if(func_name == NULL)
            {
                ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, root_node, NULL);
            }
        for(int i = 0; i < root_node->n_of_children; i++)
            {
                ast_node_ptr child = root_node->children[i];
                if(child->token.lexeme == NULL)
                    {
                        continue;
                    }

                if(strcmp(child->token.lexeme, func_name) == 0)
                    {
                        //Function
                        if(child->node_type == NT_FUNC_DECL)
                            {
                                int child_params = 0;
                                if(child->children[0]->node_type == NT_PARAM)
                                    {
                                        child_params = child->children[0]->n_of_children;
                                    }
                                if(child_params == parameter_count)
                                    {
                                        return 1;
                                    }
                            }
                        //Getter
                        else if(child->node_type == NT_GETTER && parameter_count == 0)
                            {
                                return 2;
                            }
                        //Setter
                        else if(child->node_type == NT_SETTER && parameter_count == 1)
                            {
                                return 3;
                            }
                    }
            }
        return 0;
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
                        var = var->next;
                    }
            }
        //If I don't find it in the stack, I give it the GF@ prefix
        sprintf(buffer, "GF@%s", var_name);
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
//
//List, to store and define variables
//

// Helper to save generated ID
void save_var_id(ast_node_ptr node, char *gen_id) 
    {
        id_list_item *new_item = malloc(sizeof(id_list_item));
        if (new_item) 
            {
                new_item->node = node;
                new_item->generated_id = malloc(strlen(gen_id) + 1);
                if(new_item->generated_id) 
                    {
                        strcpy(new_item->generated_id, gen_id);
                    }
                new_item->next = saved_ids;
                saved_ids = new_item;
            }
    }

// Find existing ID for node
char* find_var_id(ast_node_ptr node) 
    {
        id_list_item *current = saved_ids;
        while (current != NULL) 
            {
                if (current->node == node) 
                    {
                        return current->generated_id;
                    }
                current = current->next;
            }
        return NULL;
    }

// Cleanup list
void cleanup_ids() 
    {
        while (saved_ids != NULL) 
            {
                id_list_item *next = saved_ids->next;
                if(saved_ids->generated_id) 
                    {
                        free(saved_ids->generated_id);
                    }
                free(saved_ids);
                saved_ids = next;
            }
    }

// Manually register variable to stack without generating new ID
void stack_push_existing(char *var_name, char *unique_id, ast_node_ptr node) 
    {
        if (!var_name || !unique_id || stack.top == -1) return;
        
        var_node *var = malloc(sizeof(var_node));
        if (var == NULL) ast_error(ERROR_GEN_INTERNAL, MSG_GEN_INTERNAL, node, NULL);

        var->original_id = var_name;
        var->new_id = malloc(strlen(unique_id) + 1);
        strcpy(var->new_id, unique_id);
        
        var->next = stack.arr[stack.top];
        stack.arr[stack.top] = var;
    }

// First pass scanner - finds variables and prints DEFVAR
void scan_variables(ast_node_ptr node) 
    {
        if (node == NULL) return;

        // Skip nested functions
        if (node->node_type == NT_FUNC_DECL || node->node_type == NT_GETTER || node->node_type == NT_SETTER) 
            {
                return; 
            }

        // Handle scoping for scan pass
        bool is_block = (node->token.type == TT_LBRACE || node->node_type == NT_BLOCK || 
                        node->node_type == NT_IF_BODY || node->node_type == NT_ELSE_BODY || 
                        node->node_type == NT_WHILE_BODY);
        
        if (is_block) 
            {
                stack_push(node); 
            }

        // Found variable definition
        if (node->token.type == TT_KEYWORD_VAR || node->node_type == NT_VAR_DEF) 
            {
                char *vname = NULL;
                if (node->n_of_children > 0 && node->children != NULL && node->children[0] != NULL) 
                    {
                        vname = node->children[0]->token.lexeme;
                    } 
                else 
                    {
                        vname = node->token.lexeme;
                    }
                
                if (vname && !global_check(vname)) 
                    {
                        char *uid = stack_register_var(vname, node); 
                        if (uid) 
                            {
                                printf("DEFVAR LF@%s\n", uid);
                                save_var_id(node, uid);
                            }
                    }
            }

        if (node->children != NULL) 
            {
                for (int i = 0; i < node->n_of_children; i++) 
                    {
                        scan_variables(node->children[i]);
                    }
            }

        if (is_block) 
            {
                stack_pop();
            }
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

void print_read(char* var, char* type)
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
