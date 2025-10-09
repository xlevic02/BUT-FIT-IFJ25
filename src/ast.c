//implementace abstraktního syntaktického stromu
//AST by Jan Špaček <xspacej00> on 09/10/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "scanner.h"



void create_ast(){
    //TODO
}

ast_node_ptr ast_regular_node(ast_node_ptr current_node, ast_node_ptr previous_node){
    if (current_node == NULL){
        //TODO error handling
    }

    int n_of_children = 0; //number of children of the current node
    token_t current_token;
    while((current_token = get_token()) =! NULL){//get the next token from the scanner
        
        switch(current_token.type){
            case TT_EOF:
                return NULL;
            case TT_ERROR:
                //TODO error handling
                break;

            case TT_IDENTIFIER:
                ast_arithmetic_node();
                break;

            case TT_INT:

            case TT_FLOAT:

            case TT_STRING:

            case TT_KEYWORD_CLASS:

            case TT_KEYWORD_IF:

            case TT_KEYWORD_ELSE:

            case TT_KEYWORD_IS:

            case TT_KEYWORD_RETURN:

            case TT_KEYWORD_VAR:
                ast_node_ptr new_node = ast_create_node(current_token);
                new_node->token = get_token();
                if (new_node->token.type =! TT_IDENTIFIER){
                    //TODO error handling?
                }
                current_node->children = realloc(current_node->children, sizeof(ast_node_ptr) * (++n_of_children));
                current_node->children[n_of_children - 1] = new_node;
                break; 



            case TT_KEYWORD_STATIC:
                ast_node_ptr new_node = ast_create_node(current_token);
                current_node->children = realloc(current_node->children, sizeof(ast_node_ptr) * (++n_of_children));
                current_node->children[n_of_children - 1] = new_node;
                ast_regular_node(new_node, NULL);
                break;

            case TT_KEYWORD_IMPORT:

            case TT_KEYWORD_FOR:
            case TT_KEYWORD_WHILE:

            case TT_KEYWORD_NUM:

            case TT_KEYWORD_Null:


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





            case TT_KEYWORD_IFJ:
            case TT_LPAREN:
            case TT_RPAREN:
            case TT_LBRACE:
            case TT_RBRACE:
            case TT_COMMA:
            case TT_DOT:
                break;




            case TT_EOL:
                return current_node;
                //TODO
            default:
                //TODO error handling
                break;
        }
    }
}


ast_node_ptr ast_arithmetic_node(){
    //TODO
}


ast_node_ptr ast_create_node(token_t token){
    ast_node_ptr new_node = malloc(sizeof(ast_node_t));
    new_node->token = token;
    new_node->value.int_value = 0;
    new_node->children = NULL;
    return new_node;
    //TODO
}