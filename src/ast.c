// Implementace prekladace imperativniho jazyka IFJ25
// AST by: Jan "xspacej00" Špaček on 10/09/2025
//         Jan Frantisek "xlevic02" Levicek on 11/10/2025

#include "ast.h"

/*
 * Creates root of AST from code frame
 *
 *      Root ("Program")
 *      ├── FunctionDeclaration ("main")
 *      ├── FunctionDeclaration ("foo")
 *      └── ...
*/
ast_node_ptr create_ast(){
    //declare and initialize the root node with the first token
    token_t *current_token = malloc(sizeof(token_t));
    *current_token = get_token();       //get first token

    //prologue handling
    ast_skip_EOL(current_token);

    if (ast_handle_prologue(current_token)){
        free(current_token);
        error(ERROR_SYNTAX, MSG_SYN_MISSING_PROLOG);
    }

    ast_skip_EOL(current_token);


    if (current_token->type != TT_KEYWORD_CLASS ||
        strcmp((*current_token = get_token()).lexeme , "Program"))
    {
        free(current_token);
        error(ERROR_SYNTAX, MSG_SYN_PROGRAM_DECLARATION);
    }

    ast_node_ptr root = ast_create_node(*current_token, NULL, NT_ROOT);
    

    if ((*current_token = get_token()).type != TT_LBRACE){

        ast_error(ERROR_SYNTAX, MSG_SYN_BRACE_WRONG, root, current_token);
    }

    if((*current_token = get_token()).type != TT_EOL){
        ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, root, current_token);
    }

    //Main loop to handle all top-level constructs (functions, main)
    do{

        if(current_token->type == TT_EOL){
            ast_skip_EOL(current_token);
        }

        if (current_token->type == TT_RBRACE){
            break;
        }
        
        if (current_token->type != TT_KEYWORD_STATIC){
                ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);

        }
        
        if((*current_token = get_token()).type != TT_IDENTIFIER){
            ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_IDENTIFIER, root, current_token);
        }

        ast_node_ptr new_node = ast_create_node(*current_token, root, NT_FUNC_DECL);
        ast_increase_children(root, new_node);

        bool is_setter = false;
        if ((*current_token = get_token()).type == TT_ASSIGN){
            is_setter = true;
            *current_token = get_token();
            new_node->node_type = NT_SETTER;
        }
        if (current_token->type == TT_LPAREN){
            ast_increase_children(new_node, ast_parameter_node(current_token, new_node));
            if((*current_token = get_token()).type != TT_LBRACE){
                ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);
            }
        }else if(is_setter){
            ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);
        }else if(current_token->type != TT_LBRACE){
            ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);
        }else{
            new_node->node_type = NT_GETTER;
        }
        //TODO handle getters/setters, potentially by a specific first node?


        if((*current_token = get_token()).type != TT_EOL){
            ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, root, current_token);
        }

        ast_node_ptr body_node = ast_create_node(*current_token, new_node, NT_BLOCK);
        body_node->token.type = TT_LBRACE;
        body_node->token.lexeme = "Block";
        ast_increase_children(new_node, body_node);
        ast_regular_node(body_node, new_node, current_token, 0);
        
    } while (((*current_token = get_token()).type != TT_EOF) || (current_token->type != TT_RBRACE));;

    if(current_token->type != TT_RBRACE){
        ast_error(2, MSG_SYN_BRACE_WRONG, root, current_token);
    }
    *current_token = get_token();
    ast_skip_EOL(current_token);
    if(current_token->type != TT_EOF){
        ast_error(2, MSG_SYN_MISSING_TOKEN, root, current_token);
    }

    free(current_token);
    return root;
}




//Function to handle whole regular nodes (if, while, return, var, expressions, etc.)
ast_node_ptr ast_regular_node(ast_node_ptr current_node, ast_node_ptr previous_node, token_t *current_token, int n_of_children_initial){
    
    if (current_node == NULL){
        ast_error(99, MSG_INT_MISSING_TOKEN, previous_node, current_token);
    }

    current_node->n_of_children = n_of_children_initial;

    //Main loop to handle all constructs inside a regular node
    do{
        if(current_token->type == TT_EOL){
            ast_skip_EOL(current_token);
        }

        switch(current_token->type){
            //Arithmetic expressions
            case TT_LPAREN:
            case TT_INT:
            case TT_FLOAT:
            case TT_STRING:
            case TT_IDENTIFIER:{
                        ast_increase_children(current_node, NULL);
                        ast_expression_node(current_token, current_node);

                token_type_t control_token = current_node->children[current_node->n_of_children - 1]->token.type;
                if(control_token == TT_IDENTIFIER || control_token == TT_INT || control_token == TT_FLOAT || control_token == TT_STRING || control_token == TT_LPAREN){
                    ast_error(ERROR_SYNTAX, "Syntax error:\texpressionless literal or numeral\n", previous_node, current_token);
                }
                
                break;}




            //Return statement
            case TT_KEYWORD_RETURN:{
                ast_node_ptr new_node = ast_create_node(*current_token, current_node, NT_RETURN);
                new_node->token = *current_token;
                ast_increase_children(current_node, new_node);
                *current_token = get_token();
                if(current_token->type != TT_EOL){
                    ast_increase_children(new_node, NULL);
                    ast_expression_node(current_token, new_node);

                }

                //*current_token = get_token();
                break;}



                
            //Variable declaration
            case TT_KEYWORD_VAR:{
                *current_token = get_token();
                ast_node_ptr new_node = ast_create_node(*current_token, current_node, NT_VAR_DEF);
                if (new_node->token.type != TT_IDENTIFIER){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                ast_increase_children(current_node, new_node);

                *current_token = get_token();
                break; }





            //If statement
            case TT_KEYWORD_IF:{
                ast_node_ptr if_node = ast_create_node(*current_token, current_node, NT_IF_STATEMENT);
                ast_increase_children(current_node, if_node);
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                ast_skip_EOL(current_token);

                //Possible TODO: condition handling
                ast_increase_children(if_node, NULL);
                ast_expression_node(current_token, if_node);

                //Handle closing parenthesis, opening brace and EOL
                if (current_token->type != TT_RPAREN ||
                    (*current_token = get_token()).type != TT_LBRACE ||
                    (*current_token = get_token()).type != TT_EOL) {
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                //Handle if body
                ast_node_ptr if_body_node = ast_create_node(*current_token, if_node, NT_IF_BODY);
                if_body_node->token.type = TT_LBRACE;
                if_body_node->token.lexeme = "Block";
                ast_increase_children(if_node, if_body_node);
                ast_regular_node(if_body_node, if_node, current_token, 0);

                *current_token = get_token();

                //Else handling
                if (current_token->type != TT_KEYWORD_ELSE){
                    break; 
                }

                ast_node_ptr else_node = ast_create_node(*current_token, current_node, NT_ELSE_BODY);
                ast_increase_children(if_node, else_node);

                //Handle opening brace and EOL
                if ((*current_token = get_token()).type != TT_LBRACE || (*current_token = get_token()).type != TT_EOL){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                //Handle else body
                ast_regular_node(else_node, current_node, current_token, 0);

                *current_token = get_token();
                break;}




            //case TT_KEYWORD_FOR: possible TODO
            

            //While loop
            case TT_KEYWORD_WHILE:{
                ast_node_ptr while_node = ast_create_node(*current_token, current_node, NT_WHILE);
                ast_increase_children(current_node, while_node);
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                ast_skip_EOL(current_token);
                

                ast_increase_children(while_node, NULL);
                ast_expression_node(current_token, while_node);

                //Handle closing parenthesis, opening brace and EOL
                if(current_token->type != TT_LBRACE || (*current_token = get_token()).type != TT_EOL){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }

                //Handle while body
                ast_node_ptr while_body_node = ast_create_node(*current_token, while_node, NT_WHILE_BODY);
                while_body_node->token.type = TT_LBRACE;
                while_body_node->token.lexeme = "Block";
                ast_increase_children(while_node, while_body_node);
                ast_regular_node(while_body_node, while_node, current_token, 0);

                *current_token = get_token();
                break;}
                
                





            //Inherent function call
            case TT_KEYWORD_IFJ:
                ast_increase_children(current_node, ast_ifj_function_call_node(current_token, current_node));
                break;


                //End of a block
            case TT_RBRACE:
                return current_node;
            
            case TT_LBRACE:{
                ast_node_ptr sub_node  = ast_create_node(*current_token, current_node, NT_BLOCK);
                *current_token = get_token();
                ast_increase_children(current_node, sub_node);
                ast_regular_node(sub_node, current_node, current_token, 0);
                *current_token = get_token();
                break;
            }

                //End of a line, continue
            case TT_EOL:
                break;

                //should not happen
            default:
                ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                break;
        }//switch

        //Ensure each statement ends with an EOL
        if (current_token->type != TT_EOL){
            ast_error(2, MSG_SYN_MISSING_EOL, current_node, current_token);
        }

    } while ((*current_token = get_token()).type != TT_ERROR);
    return NULL;
}

ast_node_ptr ast_ifj_function_call_node(token_t *current_token, ast_node_ptr current_node){

    ast_node_ptr inherent_function_node = ast_create_node(*current_token, current_node, NT_BUILTIN);

    if ((*current_token = get_token()).type != TT_DOT){
        ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
    }  

    *current_token = get_token();
    ast_skip_EOL(current_token);

    if (current_token->type != TT_IDENTIFIER){
        ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
    }


    ast_node_ptr id_node = ast_create_node(*current_token, inherent_function_node, NT_ID);
    ast_increase_children(inherent_function_node, id_node);


    *current_token = get_token();
    if (current_token->type != TT_LPAREN){
        ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
    }

    ast_node_ptr parameter_node = ast_create_node(*current_token, inherent_function_node, NT_PARAM);
    ast_increase_children(inherent_function_node, parameter_node);
    *current_token = get_token();


    bool comma_present = true;
    //Argument handling
    while(current_token->type != TT_RPAREN){

        if (current_token->type == TT_COMMA){
            if (comma_present){
                ast_error(2, "Syntax error:\tmultiple commas in inherent function call\n", current_node, current_token);
            }
            *current_token = get_token();
            comma_present = true;
        }

        //Only identifiers, literals and Null are allowed as arguments
        if (current_token->type != TT_IDENTIFIER && current_token->type != TT_INT && current_token->type != TT_FLOAT && current_token->type != TT_STRING && current_token->type != TT_KEYWORD_Null){
            ast_error(2, "Syntax error:\twrong token in inherent function call\n", current_node, current_token);
        }
        ast_node_type_t node_type = NT_LITERAL;
        if (current_token->type == TT_IDENTIFIER){
            node_type = NT_ID;
        }
        
        ast_increase_children(parameter_node, ast_create_node(*current_token, current_node, node_type));
        *current_token = get_token();
        comma_present = false;
    }

    *current_token = get_token();
    return inherent_function_node;
}

//Function to handle expressions with operator precedence
ast_node_ptr ast_expression_node(token_t *current_token, ast_node_ptr parent_node){
    ast_node_ptr temp = ast_expression_inner(current_token, 0, parent_node, &(parent_node->children[parent_node->n_of_children - 1]));
    if (temp == NULL || temp->token.type == TT_ERROR){
        free_ast(&temp);
        ast_error(2, MSG_SYN_EXPRESSION, parent_node, current_token);
    }
    return temp;
}



//Helper function to handle expression parsing with operator precedence
ast_node_ptr ast_expression_inner(token_t *current_token, int min_precedence, ast_node_ptr parent_node, ast_node_ptr *upper_pointer){
    
    ast_node_ptr lhs = parse_primary(current_token, parent_node, upper_pointer);
    if (lhs == NULL || lhs->token.type == TT_ERROR){
        return lhs;
    }
    
    while (ast_get_precedence(current_token->type) != -1 &&
           ast_get_precedence(current_token->type) >= min_precedence) {
            
        token_t op_token = *current_token;
        ast_node_type_t node_type = NT_ASSIGN;
            switch (op_token.type) {
                case TT_PLUS:
                case TT_MINUS:
                case TT_MUL:
                case TT_DIV:
                    node_type = NT_AR_EXPR;
                    break;
                case TT_EQ:
                case TT_NEQ:
                case TT_LT:
                case TT_GT:
                case TT_LE:
                case TT_GE:
                case TT_KEYWORD_IS:
                    node_type = NT_BOOL_EXPR;
                    break;
                case TT_ASSIGN:
                    node_type = NT_ASSIGN;
                    break;
                default:
                    //should not happen
                    break;
            }
        int precedence = ast_get_precedence(op_token.type);
        int next_min_prec = precedence + 1;

        *current_token = get_token(); // consume operator

        ast_node_ptr op_node = ast_create_node(op_token, parent_node, node_type);
        *upper_pointer = op_node;
        ast_increase_children(op_node, lhs);
        lhs->parent = op_node;
        ast_increase_children(op_node, NULL);
        ast_expression_inner(current_token, next_min_prec, op_node, &(op_node->children[1]));


        lhs = op_node;
    }
    return lhs;
} 

//Helper function to parse primary expressions
ast_node_ptr parse_primary(token_t *current_token, ast_node_ptr parent_node, ast_node_ptr *upper_pointer){
    ast_node_ptr node = NULL;

    switch (current_token->type) {
        case TT_KEYWORD_IFJ:
            node = ast_ifj_function_call_node(current_token, parent_node);
            *upper_pointer = node;
            break;
        case TT_INT:
        case TT_FLOAT:
        case TT_STRING:
        case TT_KEYWORD_NUM:
        case TT_KEYWORD_Null:
        case TT_NULL:
            ast_node_type_t node_type = NT_LITERAL;
            if (current_token->type == TT_KEYWORD_NUM){
                node_type = NT_DATATYPE;
            }
            node = ast_create_node(*current_token, parent_node, node_type);
            *upper_pointer = node;
            *current_token = get_token(); // consume
            break;
            
        case TT_IDENTIFIER:
            node = ast_create_node(*current_token, parent_node, NT_ID);
            *upper_pointer = node;
            *current_token = get_token(); // consume
            if (current_token->type == TT_LPAREN) {
                ast_increase_children(node, ast_parameter_node(current_token, node));
                *current_token = get_token(); // consume ')'
            }
            break;

        case TT_LPAREN:{
            *current_token = get_token(); // consume '('
            node = ast_expression_inner(current_token, 0, parent_node, upper_pointer);

            if (current_token->type != TT_RPAREN) {
                ast_error(2, MSG_SYN_TOKEN_ORDER, parent_node, current_token);
            }
            *current_token = get_token(); // consume ')'
            break;}

        default:
            ast_error(2, MSG_SYN_WRONG_TOKEN_EXPRESSION, parent_node, current_token);
    }
    
    return node;
}



ast_node_ptr ast_parameter_node(token_t *current_token, ast_node_ptr parent_node){

    ast_node_ptr parameter_node =  ast_create_node(*current_token, parent_node, NT_PARAM);
    *current_token = get_token();
    
    if(current_token->type == TT_RPAREN){
        return parameter_node; //no parameters
    }
    ast_skip_EOL(current_token);
    bool comma_present = true;
    do{
        switch (current_token->type)
        {
        case TT_INT:
        case TT_FLOAT:
        case TT_STRING:
        case TT_KEYWORD_Null:
        case TT_IDENTIFIER:
            if(!comma_present){
                ast_error(ERROR_SYNTAX, "Syntax error:\tmissing comma in parameters\n", parent_node, current_token);
            }
            ast_node_type_t node_type = NT_DATATYPE;
            if (current_token->type == TT_IDENTIFIER){
                node_type = NT_ID;
            }else if(current_token->type == TT_KEYWORD_Null){
                node_type = NT_LITERAL;
            }
            comma_present = false;
            ast_node_ptr subparameter_node = ast_create_node(*current_token, parameter_node, node_type);
            ast_increase_children(parameter_node, subparameter_node);
            break;

        case TT_COMMA:
            if (comma_present){
                ast_error(2,"Syntax error:\tmultiple commas in parameter\n", parent_node,current_token);
            }
            comma_present = true;
            ast_skip_EOL(current_token);
            break;
        
        default:
            ast_error(2, "Syntax error:\tillegal token in parameters\n", parent_node, current_token);
            break;
        }
    }while((*current_token = get_token()).type != TT_RPAREN);
    if (comma_present){
        ast_error(2, "Syntax error:\ttrailing comma in parameters\n", parent_node, current_token);
    }

    return parameter_node;
}


//Helper function to skip possible EOL tokens
void ast_skip_EOL(token_t *current_token){
    while(current_token->type == TT_EOL){
        *current_token = get_token();
    }
}


int ast_handle_prologue(token_t *current_token){

    if(current_token->type != TT_KEYWORD_IMPORT) return 1;

    *current_token = get_token();
    ast_skip_EOL(current_token);

    if(strcmp(current_token->lexeme, "ifj25")) return 1;

    *current_token = get_token();

    if(current_token->type == TT_EOL) return 1;

    if(current_token->type != TT_KEYWORD_FOR) return 1;

    *current_token = get_token();
    ast_skip_EOL(current_token);

    if(current_token->type != TT_KEYWORD_IFJ) return 1;
    *current_token = get_token();

    return 0;
}


ast_node_ptr ast_create_node(token_t token, ast_node_ptr parent, ast_node_type_t node_type){
    ast_node_ptr new_node = malloc(sizeof(ast_node_t));
    if(new_node == NULL)
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, parent, &token);

    new_node->parent = parent;
    new_node->children = NULL;
    new_node->node_type = node_type;

    // Deep copy the token - allocate new memory for lexeme
    new_node->token.type = token.type;
    if (token.lexeme != NULL) {
        new_node->token.lexeme = malloc(strlen(token.lexeme) + 1);
        if (new_node->token.lexeme == NULL) {
            free(new_node);
            ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, parent, &token);
        }
        strcpy(new_node->token.lexeme, token.lexeme);
    } else {
        new_node->token.lexeme = NULL;
    }

    new_node->value.int_value = 0;
    new_node->n_of_children = 0;
    return new_node;
}


void ast_increase_children(ast_node_ptr current_node, ast_node_ptr new_node){
    if (current_node == NULL) return;

    size_t new_count = (size_t)current_node->n_of_children + 1;
    ast_node_ptr *tmp = realloc(current_node->children, sizeof(ast_node_ptr) * new_count);
    if (tmp == NULL){
        /* realloc failed: keep state unchanged, report error */
        free_ast(&new_node);
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, current_node->parent, &current_node->token);
        return; /* ast_error may exit; keep the explicit return */
    }

    current_node->children = tmp;
    current_node->children[new_count - 1] = new_node;
    current_node->n_of_children = (int)new_count;
    if(new_node != NULL) new_node->parent = current_node;
}



void ast_error(int err_num, const char* err_message, ast_node_ptr node, token_t *current_token) {
    free(current_token);
    destroy_ast(node);
    error(err_num, err_message);
}

void destroy_ast(ast_node_ptr node){
    if (node == NULL) return;
    while(node->parent != NULL){
        node = node->parent;
    }
    free_ast(&node);
}

void free_ast(ast_node_ptr *node){
    if (*node == NULL) {
        return;
    }
    if ((*node)->children != NULL){
        for (int i = 0; i < (*node)->n_of_children; i++){
            free_ast(&(*node)->children[i]);
            (*node)->children[i] = NULL;
        }
        free((*node)->children);
        (*node)->children = NULL;
    }
    free(*node);
    *node = NULL;
}


int ast_get_precedence(token_type_t type) {
    switch (type) {
        case TT_MUL:
        case TT_DIV:
            return 5;
        case TT_PLUS:
        case TT_MINUS:
            return 4;
        case TT_LT:
        case TT_GT:
        case TT_LE:
        case TT_GE:
        case TT_ASSIGN:
            return 3;
        case TT_KEYWORD_IS:
            return 2;
        case TT_EQ:
        case TT_NEQ:
            return 1;
        default:
            return -1; // not an operator
    }
}