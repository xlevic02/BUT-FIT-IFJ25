//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 09/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "scanner.h"
#include "error.h"


//Main function to create the AST from tokens
ast_node_ptr create_ast(){
    //declare and initialize the root node with the first token
    token_t *current_token = malloc(sizeof(token_t));
    *current_token = get_token();       //get first token
    ast_node_ptr root = ast_create_node(*current_token, NULL);
    int n_of_children = 0;              //number of children of the root node

    //prologue handling
    ast_skip_EOL(current_token);

    if (ast_handle_prologue(current_token)){
        ast_error(2, MSG_SYN_MISSING_PROLOG, root, current_token);
    }

    ast_skip_EOL(current_token);


    if (current_token->type != TT_KEYWORD_CLASS && strcmp((*current_token = get_token()).lexeme , "Program") && (*current_token = get_token()).type != TT_LBRACE){
        ast_error(2, "Syntax error:\tincorrect Program declaration\n", root, current_token);
    }

    //Main loop to handle all top-level constructs (functions, main)
    do{
        switch (current_token->type){
            //Skip empty lines
            case TT_EOL:
                ast_skip_EOL(current_token);
                break;
            
            //Function handling
            case TT_KEYWORD_STATIC:
                if((*current_token = get_token()).type != TT_IDENTIFIER){
                    ast_error(2, MSG_SYN_MISSING_TOKEN, root, current_token);
                }

                ast_node_ptr new_node = ast_create_node(*current_token, root);
                ast_increase_children(root, new_node, ++n_of_children);

                new_node->children[0] = ast_parameter_node(current_token, new_node);

                ast_regular_node(new_node, root, current_token, 1);
                break;
            
            //Fail state of incompatible tokens
            default:
                //should not happen
                ast_error(2, "Syntax error: illegal token in root", root, current_token);
                break;
        }
        
    } while (((*current_token = get_token()).type != TT_EOF) && (current_token->type != TT_LBRACE));;

    if(current_token->type != TT_RBRACE){
        ast_error(2, MSG_SYN_MISSING_TOKEN, root, current_token);
    }

    free(current_token);
    root->value.int_value = n_of_children;
    return root;
}




//Function to handle whole regular nodes (if, while, return, var, expressions, etc.)
ast_node_ptr ast_regular_node(ast_node_ptr current_node, ast_node_ptr previous_node, token_t *current_token, int n_of_children_initial){
    
    if (current_node == NULL){
        ast_error(99, MSG_INT_MISSING_TOKEN, previous_node, current_token);
    }

    int n_of_children = n_of_children_initial;

    //Main loop to handle all constructs inside a regular node
    do{

        switch(current_token->type){

            //Fail state of incompatible tokens
            case TT_ERROR:
                ast_error(1, MSG_LEX_PROHIBITED_CHAR, current_node, current_token);
                break;


            //Arithmetic expressions
            case TT_LPAREN:
            case TT_INT:
            case TT_FLOAT:
            case TT_STRING:
            case TT_IDENTIFIER:
                ast_increase_children(current_node, ast_arithmetic_node(current_token), ++n_of_children);

                *current_token = get_token();
                break;




            //Return statement
            case TT_KEYWORD_RETURN:
                ast_node_ptr new_node = ast_create_node(*current_token, current_node);
                new_node->token = *current_token;
                ast_increase_children(current_node, new_node, ++n_of_children);
                *current_token = get_token();
                if(current_token->type != TT_EOL){
                    new_node->children[0] = malloc(sizeof(ast_node_ptr));
                    new_node->children[0] = ast_arithmetic_node(current_token);
                }

                *current_token = get_token();
                break;



                
            //Variable declaration
            case TT_KEYWORD_VAR:
                ast_node_ptr new_node = ast_create_node(*current_token, current_node);
                new_node->token = get_token();
                if (new_node->token.type != TT_IDENTIFIER){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                ast_increase_children(current_node, new_node, ++n_of_children);

                *current_token = get_token();
                break; 





            //If statement
            case TT_KEYWORD_IF:
                ast_node_ptr if_node = ast_create_node(*current_token, current_node);
                ast_increase_children(current_node, if_node, ++n_of_children);
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                ast_skip_EOL(current_token);

                //Possible TODO: condition handling
                if_node->children = malloc(sizeof(ast_node_ptr));
                if_node->children[0] = ast_arithmetic_node(current_token);

                //Handle closing parenthesis, opening brace and EOL
                if((*current_token = get_token()).type != TT_RPAREN && (*current_token = get_token()).type != TT_LBRACE && (*current_token = get_token()).type != TT_EOL){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                //Handle if body
                ast_regular_node(if_node, current_node, current_token, 1);

                *current_token = get_token();

                //Else handling
                if (current_token->type != TT_KEYWORD_ELSE){
                    break; 
                }

                ast_node_ptr else_node = ast_create_node(*current_token, current_node);
                ast_increase_children(current_node, else_node, ++n_of_children);

                //Handle opening brace and EOL
                if ((*current_token = get_token()).type != TT_LBRACE && (*current_token = get_token()).type != TT_EOL){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                //Handle else body
                ast_regular_node(else_node, current_node, current_token, 0);

                *current_token = get_token();
                break;




            //case TT_KEYWORD_FOR: possible TODO
            

            //While loop
            case TT_KEYWORD_WHILE:
                ast_node_ptr while_node = ast_create_node(*current_token, current_node);
                ast_increase_children(current_node, while_node, ++n_of_children);
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                ast_skip_EOL(current_token);

                //Possible TODO: condition handling
                while_node->children = malloc(sizeof(ast_node_ptr));
                while_node->children[0] = ast_arithmetic_node(current_token);

                //Handle closing parenthesis, opening brace and EOL
                if((*current_token = get_token()).type != TT_RPAREN && (*current_token = get_token()).type != TT_LBRACE && (*current_token = get_token()).type != TT_EOL){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                //Handle while body
                ast_regular_node(while_node, current_node, current_token, 1);

                *current_token = get_token();
                break;
                
                





            //Inherent function call
            case TT_KEYWORD_IFJ:

                if ((*current_token = get_token()).type != TT_DOT){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }  

                *current_token = get_token();
                ast_skip_EOL(current_token);

                if (current_token->type != TT_IDENTIFIER){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }

                ast_node_ptr inherent_function_node = ast_create_node(*current_token, current_node);
                ast_increase_children(current_node, inherent_function_node, ++n_of_children);


                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                }
                *current_token = get_token();

                //Argument handling
                while(current_token->type != TT_RPAREN){

                    if (current_token->type == TT_COMMA){
                        *current_token = get_token();
                    }

                    //Only identifiers, literals and Null are allowed as arguments
                    if (current_token->type != TT_IDENTIFIER && current_token->type != TT_INT && current_token->type != TT_FLOAT && current_token->type != TT_STRING && current_token->type != TT_KEYWORD_Null){
                        ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                    }
                    
                    ast_increase_children(inherent_function_node, ast_create_node(*current_token, current_node), ++n_of_children);
                    *current_token = get_token();
                }

                *current_token = get_token();
                break;


                //End of a block
            case TT_RBRACE:
                current_node->value.int_value = n_of_children;
                return current_node;

                //End of a line, continue
            case TT_EOL:
                break;


            /*    //Should not happen
            case TT_KEYWORD_NUM:
            case TT_RPAREN:
            case TT_COMMA:
            case TT_DOT:
            case TT_LBRACE:
            case TT_KEYWORD_ELSE:
            case TT_KEYWORD_IS:
            case TT_ASSIGN:
            case TT_PLUS:
            case TT_MINUS:
            case TT_MUL:
            case TT_DIV:
            case TT_EQ:
            case TT_NEQ:
            case TT_LT:
            case TT_GT:
            case TT_LE:
            case TT_GE:
            case TT_KEYWORD_CLASS:
            case TT_KEYWORD_IMPORT:
            case TT_EOF:
            case TT_KEYWORD_STATIC:*/

                //should not happen
            default:
                ast_error(2, MSG_SYN_TOKEN_ORDER, current_node, current_token);
                break;
        }

        //Ensure each statement ends with an EOL
        if (current_token->type != TT_EOL){
            ast_error(2, MSG_SYN_MISSING_EOL, current_node, current_token);
        }
        //skip all EOLs
        ast_skip_EOL(current_token);

    } while ((*current_token = get_token()).type != TT_ERROR);
}




ast_node_ptr ast_arithmetic_node(token_t *current_token){
    //TODO
} 



ast_node_ptr ast_parameter_node(token_t *current_token, ast_node_ptr parent_node){
    if ((*current_token = get_token()).type != TT_LPAREN){
        ast_error(2, "Syntax error:\tmissing left parentheses in parameters\n", parent_node, current_token);
    }
    ast_skip_EOL(current_token);
    ast_node_ptr parameter_node =  ast_create_node(*current_token, parent_node);
    bool comma_present = true;
    int n_of_children = 0;
    while((*current_token = get_token()).type != TT_RPAREN){
        switch (current_token->type)
        {
        case TT_IDENTIFIER:
            if(comma_present == false){
                ast_error(2, "Syntax error:\tmissing comma in parameters\n", parent_node, current_token);
            }
            comma_present = false;
            ast_node_ptr subparameter_node = ast_create_node(*current_token, parameter_node);
            ast_increase_children(parameter_node, subparameter_node, ++n_of_children);
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
    }
    parameter_node->value.int_value = n_of_children;
    return parameter_node;
}

void ast_skip_EOL(token_t *current_token){
    while(current_token->type == TT_EOL){
        *current_token = get_token();
    }
}


int ast_handle_prologue(token_t *current_token){
    if (current_token->type != TT_KEYWORD_CLASS && strcmp((*current_token = get_token()).lexeme, "ifj25") && (*current_token = get_token()).type != TT_KEYWORD_FOR && (*current_token = get_token()).type != TT_KEYWORD_IFJ){
        return 1;
    }
    return 0;
}


ast_node_ptr ast_create_node(token_t token, ast_node_ptr parent){
    ast_node_ptr new_node = malloc(sizeof(ast_node_t));
    new_node->token = token;
    new_node->value.int_value = 0;
    new_node->children = NULL;
    new_node->parent = parent;
    return new_node;
}

void ast_increase_children(ast_node_ptr current_node, ast_node_ptr new_node, int n_of_children){
    current_node->children = realloc(current_node->children, sizeof(ast_node_ptr) * n_of_children);
    current_node->children[n_of_children - 1] = new_node;
}





void ast_error(int err_num, const char* err_message, ast_node_ptr node, token_t *current_token) {
    free(current_token);
    destroy_ast(node);
    fprintf(stderr,err_message);
    exit(err_num);
}

void destroy_ast(ast_node_ptr node){
    if (node == NULL) return;
    while(node->parent != NULL){
        node = node->parent;
    }
    free_ast(node);
}

void free_ast(ast_node_ptr node){
    if (node == NULL) return;
    if (node->children != NULL){
        for (int i = 0; node->children[i] != NULL; i++){
            free_ast(node->children[i]);
        }
        free(node->children);
    }
    free(node);
}