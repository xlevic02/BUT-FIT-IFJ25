//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 09/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "scanner.h"



ast_node_ptr create_ast(){
    token_t *current_token = malloc(sizeof(token_t));
    *current_token = get_token();

    //TODO handling prologu
    while((*current_token = get_token()).type != TT_KEYWORD_CLASS);

    if ((*current_token = get_token()).lexeme != "Pogram"){
        //TODO error handling
    }
    
    ast_node_ptr root = ast_create_node(*current_token);
    int n_of_children = 1; //number of children of the root node
    root->children = malloc(sizeof(ast_node_ptr));
    do{
        switch (current_token->type)
        {
        case TT_EOL:
            //ignore
            break;
        case TT_KEYWORD_STATIC:
            if((*current_token = get_token()).type != TT_IDENTIFIER){
                //TODO error handling
            }
            if(current_token->lexeme == "main"){
                ast_node_ptr new_node = ast_create_node(*current_token);
                //TODO check if main is already there
                root->children[0] = new_node;
                root->children[0] = ast_regular_node(new_node, NULL, 0);
            } else {
                ast_node_ptr new_node = ast_create_node(*current_token);
                root->children = realloc(root->children, sizeof(ast_node_ptr) * (++n_of_children));
                root->children[n_of_children - 1] = new_node;
                //TODO arguent handling
                root->children[n_of_children - 1] = ast_regular_node(new_node, NULL, 0);
            }
        
        default:
            //should not happen
            //TODO error handling
            break;
        }
        
    } while ((*current_token = get_token()).type != TT_EOF && current_token->type != TT_LBRACE);

    if(current_token->type != TT_LBRACE){
        //TODO error handling
    }

    free(current_token);
    return root;
}





ast_node_ptr ast_regular_node(ast_node_ptr current_node, ast_node_ptr previous_node, token_t *current_token, int n_of_children_initial){
    if (current_node == NULL){
        //TODO error handling
    }

    int *n_of_children = malloc(sizeof(int)); //number of children of the current node
    *n_of_children = n_of_children_initial;
    while(1){
        *current_token = get_token();
        switch(current_token->type){

            case TT_ERROR:
                //TODO error handling
                break;



            case TT_LPAREN:
            case TT_KEYWORD_Null:
            case TT_INT:
            case TT_FLOAT:
            case TT_STRING:
            case TT_IDENTIFIER:
                ast_increase_children(current_node, ast_arithmetic_node(current_token), n_of_children);
                break;





            case TT_KEYWORD_RETURN:
                ast_node_ptr new_node = ast_create_node(*current_token);
                new_node->token = *current_token;
                ast_increase_children(current_node, new_node, n_of_children);
                *current_token = get_token();
                if(current_token->type != TT_EOL){
                    new_node->children[0] = malloc(sizeof(ast_node_ptr));
                    new_node->children[0] = ast_arithmetic_node(current_token);
                }
                break;



                

            case TT_KEYWORD_VAR:
                ast_node_ptr new_node = ast_create_node(*current_token);
                new_node->token = get_token();
                if (new_node->token.type =! TT_IDENTIFIER){
                    //TODO error handling?
                }
                ast_increase_children(current_node, new_node, n_of_children);
                break; 






            case TT_KEYWORD_IF:
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    //TODO error handling
                }
                ast_skip_EOL(current_token);
                ast_node_ptr if_node = ast_create_node(*current_token);
                ast_increase_children(current_node, if_node, n_of_children);
                if_node->children = malloc(sizeof(ast_node_ptr));
                if_node->children[0] = ast_arithmetic_node(current_token);
                *current_token = get_token();
                if (current_token->type != TT_RPAREN){
                    //TODO error handling
                }
                *current_token = get_token();
                if (current_token->type != TT_LBRACE){
                    //TODO error handling
                }
                ast_regular_node(if_node, current_node, current_token, 1);



                ast_skip_EOL(current_token);
                if (current_token->type != TT_KEYWORD_ELSE){
                    break;
                }
                ast_node_ptr else_node = ast_create_node(*current_token);
                ast_increase_children(current_node, else_node, n_of_children);
                *current_token = get_token();
                if (current_token->type != TT_LBRACE){
                    //TODO error handling
                }
                *current_token = get_token();
                ast_regular_node(else_node, current_node, current_token, 0);
                break;




            //case TT_KEYWORD_FOR: if possible TODO
            


            case TT_KEYWORD_WHILE:
                ast_node_ptr while_node = ast_create_node(*current_token);
                ast_increase_children(current_node, while_node, n_of_children);
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    //TODO error handling
                }
                ast_skip_EOL(current_token);
                while_node->children = malloc(sizeof(ast_node_ptr));
                while_node->children[0] = ast_arithmetic_node(current_token);
                *current_token = get_token();
                if (current_token->type != TT_RPAREN){
                    //TODO error handling
                }
                *current_token = get_token();
                if (current_token->type != TT_LBRACE){
                    //TODO error handling
                }
                ast_regular_node(while_node, current_node, current_token, 1);
                break;
                
                






            case TT_KEYWORD_IFJ:
                *current_token = get_token();
                if (current_token->type != TT_DOT){
                    //TODO error handling
                }  
                ast_skip_EOL(current_token);
                if (current_token->type != TT_IDENTIFIER){
                    //TODO error handling
                }
                ast_node_ptr inherent_function_node = ast_create_node(*current_token);
                ast_increase_children(current_node, inherent_function_node, n_of_children);
                *current_token = get_token();
                if (current_token->type != TT_LPAREN){
                    //TODO error handling
                }
                *current_token = get_token();
                while(current_token->type != TT_RPAREN){
                    if (current_token->type == TT_COMMA){
                        *current_token = get_token();
                        continue;
                    }
                    if (current_token->type != TT_IDENTIFIER && current_token->type != TT_INT && current_token->type != TT_FLOAT && current_token->type != TT_STRING && current_token->type != TT_KEYWORD_Null){
                        //TODO error handling
                    }
                    ast_increase_children(inherent_function_node, ast_create_node(*current_token), n_of_children);
                    *current_token = get_token();
                }
                break;


                //End of a block
            case TT_RBRACE:
                free(n_of_children);
                return current_node;

            case TT_EOL:
                //ignore
                break;

                //???? dont know what this is
            case TT_KEYWORD_NUM:
                //Should not happen
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
            case TT_KEYWORD_STATIC:
                //TODO error handling
                break;

                //should not happen
            default:
                //TODO error handling
                break;
        }
    }
}




ast_node_ptr ast_arithmetic_node(token_t *current_token){
    //TODO
}

//TODO check if this works as intended
void ast_skip_EOL(token_t *current_token){
    while(current_token->type == TT_EOL){
        *current_token = get_token();
    }
}


ast_node_ptr ast_create_node(token_t token){
    ast_node_ptr new_node = malloc(sizeof(ast_node_t));
    new_node->token = token;
    new_node->value.int_value = 0;
    new_node->children = NULL;
    return new_node;
    //TODO
}

void ast_increase_children(ast_node_ptr current_node, ast_node_ptr new_node, int *n_of_children){
    current_node->children = realloc(current_node->children, sizeof(ast_node_ptr) * (++*n_of_children));
    current_node->children[*n_of_children - 1] = new_node;
}