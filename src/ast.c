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
    if(current_token == NULL)
        error(ERROR_INTERNAL, MSG_INT_MALLOC);

    *current_token = get_token();       //get first token
    int n_of_children = 0;              //number of children of the root node

    //prologue handling
    ast_skip_EOL(current_token);

    if (ast_handle_prologue(current_token)){
        free(current_token);
        error(ERROR_SYNTAX, MSG_SYN_MISSING_PROLOG);
    }

    ast_skip_EOL(current_token);


    if (current_token->type != TT_KEYWORD_CLASS &&
        strcmp((*current_token = get_token()).lexeme , "Program"))
    {
        free(current_token);
        error(ERROR_SYNTAX, MSG_SYN_PROGRAM_DECLARATION);
    }

    ast_node_ptr root = ast_create_node(*current_token, NULL, NT_ROOT);
    if(root == NULL) {
        free(current_token);
        error(ERROR_INTERNAL, MSG_INT_MALLOC);
    }

    if ((*current_token = get_token()).type != TT_LBRACE)
        ast_error(ERROR_SYNTAX, MSG_SYN_PROGRAM_DECLARATION, root, current_token);


    if((*current_token = get_token()).type != TT_EOL)
        ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, root, current_token);


    //Main loop to handle all top-level constructs (functions, main)
    do{
        switch (current_token->type){
            //Skip empty lines
            case TT_EOL:
                ast_skip_EOL(current_token);
                break;
            
            //Function handling
            case TT_KEYWORD_STATIC:
                if((*current_token = get_token()).type != TT_IDENTIFIER)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);



                ast_node_ptr new_node = ast_create_node(*current_token, root, NT_FUNC_DECL);
                if(new_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, root, current_token);

                if(ast_increase_children(root, new_node)) {
                    free(new_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, root, current_token);
                }

                if((*current_token = get_token()).type != TT_LPAREN)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);

                new_node->children[0] = ast_parameter_node(current_token, new_node);

                if(new_node->children[0] == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);

                if(new_node->children[0] == (ast_node_ptr) -1)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, new_node, current_token);



                if((*current_token = get_token()).type != TT_LBRACE)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);

                if((*current_token = get_token()).type != TT_EOL)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, root, current_token);


                ast_regular_node(new_node, current_token);

                if(current_token->type != TT_RBRACE)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);

                if((*current_token = get_token()).type != TT_EOL)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, root, current_token);

                break;
            
            //Fail state of incompatible tokens
            default:
                //should not happen
                ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, root, current_token);
                break;

        }//switch
        
    } while (((*current_token = get_token()).type != TT_RBRACE));

    if((*current_token = get_token()).type != TT_EOF)
        ast_error(ERROR_SYNTAX, "Syntax error:\tout of program frame\n", root, current_token);


    free(current_token);
    return root;
}




//Function to handle whole regular nodes (if, while, return, var, expressions, etc.)
void ast_regular_node(ast_node_ptr current_node, token_t *current_token){

    ast_node_ptr new_node = NULL;
    ast_node_ptr tmp_node = NULL;
    token_t* tmp_tok = NULL;

    //Main loop to handle all constructs inside a block
    while(current_token->type != TT_EOF){
        switch(current_token->type){
            //Assign
            case TT_IDENTIFIER:
                tmp_tok = current_token;
                *current_token = get_token();

                if(current_token->type != TT_ASSIGN) {
                    free(tmp_tok);
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);
                }

                new_node = ast_create_node(*current_token, current_node, NT_ASSIGN);
                if(new_node == NULL) {
                    free(tmp_tok);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);
                }

                if(ast_increase_children(current_node, new_node)) {
                    free(tmp_tok);
                    free(new_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, current_node, current_token);
                }


                tmp_node = ast_create_node(*tmp_tok, new_node, NT_ID);
                if(tmp_node == NULL) {
                    free(tmp_tok);
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, current_node, current_token);
                }

                if(ast_increase_children(new_node, tmp_node)) {
                    free(tmp_tok);
                    free(tmp_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, current_node, current_token);
                }

                free(tmp_tok);

                *current_token = get_token();
                ast_skip_EOL(current_token);

                ast_expression_node(new_node, current_token, 1);

                break;

                



            //Return statement
            case TT_KEYWORD_RETURN:
                new_node = ast_create_node(*current_token, current_node, NT_RETURN);
                if(new_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, current_node, current_token);

                if(ast_increase_children(current_node, new_node)) {
                    free(new_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, current_node, current_token);
                }


                *current_token = get_token();

                if(current_token->type != TT_EOL){
                    if (current_token->type == TT_EOF)
                        ast_error(ERROR_SYNTAX, MSG_SYN_UNEXPECTED_EOF, new_node, current_token);

                    ast_expression_node(new_node, current_token, 0);
                }

                *current_token = get_token();
                break;




                
            //Variable declaration
            case TT_KEYWORD_VAR:
                *current_token = get_token();
                if(current_token->type != TT_IDENTIFIER)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);

                new_node = ast_create_node(*current_token, current_node, NT_ID);
                if(new_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, current_node, current_token);

                if(ast_increase_children(current_node, new_node)) {
                    free(new_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, current_node, current_token);
                }


                *current_token = get_token();
                break; 





            //If statement
            case TT_KEYWORD_IF:
                new_node = ast_create_node(*current_token, current_node, NT_IF_STATEMENT);
                if(new_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, current_node, current_token);

                if(ast_increase_children(current_node, new_node)) {
                    free(new_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, current_node, current_token);
                }


                if((*current_token = get_token()).type != TT_LPAREN)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);

                *current_token = get_token();
                ast_skip_EOL(current_token);


                ast_expression_node(new_node, current_token, 0);


                //Handle opening brace and EOL
                if((*current_token = get_token()).type != TT_LBRACE)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);

                tmp_node = ast_create_node(*current_token, new_node, NT_IF_BODY);
                if(tmp_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);

                if(ast_increase_children(new_node, tmp_node)) {
                    free(tmp_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, new_node, current_token);
                }

                if((*current_token = get_token()).type != TT_EOL)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, new_node, current_token);

                ast_skip_EOL(current_token);


                //Handle if body
                ast_regular_node(tmp_node, current_token);

                *current_token = get_token();


                //Else body
                if (current_token->type != TT_KEYWORD_ELSE){
                    break; 
                }

                tmp_node = ast_create_node(*current_token, current_node, NT_ELSE_BODY);
                if(tmp_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);

                if(ast_increase_children(current_node, tmp_node)) {
                    free(tmp_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, new_node, current_token);
                }


                //Handle opening brace and EOL
                if ((*current_token = get_token()).type != TT_LBRACE && (*current_token = get_token()).type != TT_EOL){
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);
                }

                ast_skip_EOL(current_token);


                //Handle else body
                ast_regular_node(tmp_node, current_token);

                *current_token = get_token();
                break;




            //case TT_KEYWORD_FOR: possible TODO
            

            //While loop
            case TT_KEYWORD_WHILE:
                new_node = ast_create_node(*current_token, current_node, NT_WHILE);
                if(new_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, current_node, current_token);

                if(ast_increase_children(current_node, new_node)) {
                    free(new_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, current_node, current_token);
                }


                *current_token = get_token();
                if (current_token->type != TT_LPAREN)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);

                ast_skip_EOL(current_token);

                ast_expression_node(new_node, current_token,0);

                //Handle opening brace and EOL
                if((*current_token = get_token()).type != TT_LBRACE)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);

                tmp_node = ast_create_node(*current_token, new_node, NT_WHILE_BODY);
                if(tmp_node == NULL)
                    ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);

                if(ast_increase_children(new_node, tmp_node)) {
                    free(tmp_node);
                    ast_error(ERROR_INTERNAL, MSG_INT_REALLOC, new_node, current_token);
                }

                if((*current_token = get_token()).type != TT_EOL)
                    ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, new_node, current_token);

                ast_skip_EOL(current_token);


                //Handle while body
                ast_regular_node(new_node, current_token);

                *current_token = get_token();
                break;
                




                //End of a block
            case TT_RBRACE:
                *current_token = get_token();
                return;



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
            case TT_KEYWORD_STATIC:
            case TT_KEYWORD_IFJ */

                //should not happen
            default:
                ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, current_node, current_token);
                break;
        }//switch

        //Ensure each statement ends with an EOL
        if (current_token->type != TT_EOL){
            ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_EOL, current_node, current_token);
        }
        //skip all EOLs
        ast_skip_EOL(current_token);

    }
}

ast_node_ptr ast_expression_subtree(token_t *current_token, ast_node_ptr parent_node) {
    ast_node_ptr term = NULL;
    ast_node_ptr operator = NULL;
    ast_node_ptr tmp = NULL;

    term = ast_get_term(current_token, parent_node);
    if(term == NULL)
        return NULL;

    if(term == (ast_node_ptr) -1)
        return term;


    *current_token = get_token();

    while(current_token->type != TT_EOL) {

        switch (current_token->type) {
            case TT_EQ:              // ==
            case TT_NEQ:             // !=
            case TT_LT:              // <
            case TT_GT:              // >
            case TT_LE:              // <=
            case TT_GE:              // >=
            case TT_KEYWORD_IS:      // is
                operator = ast_create_node(*current_token, parent_node, NT_BOOL_EXPR);
                if(operator == NULL) {
                    destroy_ast(term);
                    return NULL;
                }

                if (ast_operator_eval(operator, parent_node, term)) {
                    free(operator);
                    destroy_ast(term);
                    return NULL;
                }

                parent_node = operator;

                break;

            case TT_PLUS:            // +
            case TT_MINUS:           // -
            case TT_MUL:             // *
            case TT_DIV:             // /
                operator = ast_create_node(*current_token, parent_node, NT_AR_EXPR);
                if(operator == NULL) {
                    destroy_ast(term);
                    return NULL;
                }

                if (ast_operator_eval(operator, parent_node, term)) {
                    free(operator);
                    destroy_ast(term);
                    return NULL;
                }

                parent_node = operator;

                break;

            case TT_LPAREN:
                if(term->node_type != NT_ID && term->node_type != NT_BUILTIN) {
                    destroy_ast(term);
                    return (ast_node_ptr) -1;
                }

                tmp = ast_parameter_node(current_token, term);
                if(tmp == NULL) {
                    destroy_ast(term);
                    return NULL;
                }

                if(tmp == (ast_node_ptr) -1) {
                    destroy_ast(term);
                    return tmp;
                }

                if(ast_increase_children(term, tmp)) {
                    destroy_ast(term);
                    return NULL;
                }

                if((*current_token = get_token()).type != TT_EOL) {
                    destroy_ast(term);
                    return (ast_node_ptr) -1;
                }

                return term;

            case TT_RPAREN:
                tmp = ast_find_top(parent_node);
                return tmp == NULL ? term : tmp;

            default:
                if(term->node_type == NT_BUILTIN)
                    return term;

                destroy_ast(term);
                return (ast_node_ptr) -1;

        }//switch

        *current_token = get_token();
        ast_skip_EOL(current_token);

        term = ast_get_term(current_token, operator);
        if(term == NULL) {
            destroy_ast(operator);
            return NULL;
        }

        if(term == (ast_node_ptr) -1) {
            destroy_ast(operator);
            return term;
        }

        if(ast_increase_children(operator, term)) {
            free(term);
            destroy_ast(operator);
            return NULL;
        }

        *current_token = get_token();
    }//while

    tmp = ast_find_top(parent_node);
    return tmp == NULL ? term : tmp;
}



ast_node_ptr ast_find_top(ast_node_ptr parent_node) {
    if(parent_node == NULL)
        return NULL;

    while(parent_node->parent != NULL)
        parent_node = parent_node->parent;

    return parent_node;
}



// Finds operators place in expression subtree based on precedence
int ast_operator_eval(ast_node_ptr operator, ast_node_ptr parent_node, ast_node_ptr term) {
    ast_node_ptr tmp = NULL;

    if(parent_node == NULL) {
        if(ast_increase_children(operator, term))
            return 1;

        term->parent = operator;

        return 0;
    } else {
        tmp = parent_node;

        while(tmp != NULL) {
            if (ast_get_precedence(operator->token.type) > ast_get_precedence(tmp->token.type)) {

                if (ast_increase_children(operator, tmp->children[1]))
                    return 1;

                tmp->children[1] = operator;
                operator->parent = tmp;

                return 0;
            } else {
                if(tmp->parent == NULL) {
                    if (ast_increase_children(operator, tmp))
                        return 1;

                    tmp->parent = operator;
                    operator->parent = NULL;

                    return 0;
                }

                tmp = tmp->parent;
            }
        }//while
    }//if
}


ast_node_ptr ast_get_term(token_t *current_token, ast_node_ptr parent_node) {
    ast_node_ptr new_node = NULL;

    switch(current_token->type) {
        case TT_IDENTIFIER:
            new_node = ast_create_node(*current_token, parent_node, NT_ID);

            return new_node;

        case TT_INT:
        case TT_FLOAT:
        case TT_STRING:
        case TT_NULL:
            new_node = ast_create_node(*current_token, parent_node, NT_LITERAL);

            return new_node;

        case TT_LPAREN:
            *current_token = get_token();
            ast_skip_EOL(current_token);
            new_node = ast_expression_subtree(current_token, NULL);
            if(current_token->type != TT_RPAREN) {
                destroy_ast(new_node);
                return (ast_node_ptr) -1;
            }

            *current_token = get_token();

            return new_node;

        case TT_KEYWORD_IFJ:
            new_node = ast_create_node(*current_token, parent_node, NT_BUILTIN);
            if(new_node == NULL)
                return new_node;

            if((*current_token = get_token()).type != TT_DOT) {
                destroy_ast(new_node);
                return (ast_node_ptr) -1;
            }

            *current_token = get_token();
            ast_skip_EOL(current_token);

            //Ifj.ID
            if(current_token->type != TT_IDENTIFIER) {
                destroy_ast(new_node);
                return (ast_node_ptr) -1;
            }

            ast_node_ptr tmp = ast_create_node(*current_token, new_node, NT_ID);
            if(tmp == NULL) {
                destroy_ast(new_node);
                return tmp;
            }

            if(ast_increase_children(new_node, tmp)) {
                destroy_ast(new_node);
                return NULL;
            }

            if((*current_token = get_token()).type != TT_LPAREN) {
                destroy_ast(new_node);
                return (ast_node_ptr) -1;
            }


            //Ifj params
            tmp = ast_parameter_node(current_token, new_node);
            if(tmp == NULL) {
                destroy_ast(new_node);
                return NULL;
            }

            if(tmp == (ast_node_ptr) -1) {
                destroy_ast(new_node);
                return tmp;
            }

            if(ast_increase_children(new_node, tmp)) {
                destroy_ast(new_node);
                return NULL;
            }

            if((*current_token = get_token()).type != TT_EOL) {
                destroy_ast(new_node);
                return (ast_node_ptr) -1;
            }

            return new_node;

        default:

            return (ast_node_ptr) -1;
    }
}





void ast_expression_node(ast_node_ptr new_node, token_t *current_token, int n_of_children) {
    new_node->children[n_of_children] = malloc(sizeof(ast_node_ptr));
    if(new_node->children[n_of_children] == NULL)
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);

    new_node->children[n_of_children] = ast_expression_subtree(current_token, NULL);
    if(new_node->children[n_of_children] == NULL)
        ast_error(ERROR_INTERNAL, MSG_INT_MALLOC, new_node, current_token);

    if(new_node->children[n_of_children] == (ast_node_ptr) -1)
        ast_error(ERROR_SYNTAX, MSG_SYN_MISSING_TOKEN, new_node, current_token);

    new_node->children[n_of_children]->parent = new_node;
}





ast_node_ptr ast_parameter_node(token_t *current_token, ast_node_ptr parent_node){
    ast_node_ptr param_id_node = NULL;
    ast_node_ptr parameter_node =  ast_create_node(*current_token, parent_node, NT_PARAM);
    if(parameter_node == NULL)
        return NULL;

    *current_token = get_token();
    ast_skip_EOL(current_token);
    bool comma_present = true;
    while(current_token->type != TT_RPAREN){

        switch (current_token->type)
        {
            case TT_IDENTIFIER:
                if(!comma_present) {
                    destroy_ast(parameter_node);
                    return (ast_node_ptr) -1;
                }


                comma_present = false;
                param_id_node = ast_create_node(*current_token, parameter_node, NT_ID);
                if(param_id_node == NULL) {
                    destroy_ast(parameter_node);
                    return NULL;
                }


                if(ast_increase_children(parameter_node, param_id_node)) {
                    destroy_ast(parameter_node);
                    return NULL;
                }

                *current_token = get_token();
                break;

            case TT_INT:
            case TT_FLOAT:
            case TT_STRING:
            case TT_NULL:
                if(!comma_present) {
                    destroy_ast(parameter_node);
                    return (ast_node_ptr) -1;
                }


                comma_present = false;
                param_id_node = ast_create_node(*current_token, parameter_node, NT_LITERAL);
                if(param_id_node == NULL) {
                    destroy_ast(parameter_node);
                    return NULL;
                }


                if(ast_increase_children(parameter_node, param_id_node)) {
                    destroy_ast(parameter_node);
                    return NULL;
                }

                *current_token = get_token();
                break;

            case TT_COMMA:
                if (comma_present) {
                    destroy_ast(parameter_node);
                    return (ast_node_ptr) -1;
                }


                comma_present = true;
                *current_token = get_token();
                ast_skip_EOL(current_token);
                break;
        
            default:
                destroy_ast(parameter_node);
                return (ast_node_ptr) -1;

        }//switch
    }//while

    if (comma_present && (parameter_node->n_of_children != 0)){
        destroy_ast(parameter_node);
        return (ast_node_ptr) -1;
    }


    return parameter_node;
}

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

    return 0;
}


ast_node_ptr ast_create_node(token_t token, ast_node_ptr parent, ast_node_type_t node_type){
    ast_node_ptr new_node = malloc(sizeof(ast_node_t));
    if(new_node == NULL)
        return NULL;

    new_node->parent = parent;
    new_node->children = NULL;
    new_node->node_type = node_type;
    new_node->token = token;
    new_node->value.int_value = 0;
    new_node->n_of_children = 0;
    return new_node;
}

int ast_increase_children(ast_node_ptr current_node, ast_node_ptr new_node){
    current_node->children = realloc(current_node->children, sizeof(ast_node_ptr) * ++current_node->n_of_children);
    if(current_node->children == NULL)
        return 1;

    current_node->children[current_node->n_of_children - 1] = new_node;
    return 0;
}



void ast_error(int err_num, const char* err_message, ast_node_ptr node, token_t *current_token) {
    free(current_token);
    destroy_ast(node);
    error(err_num, err_message);
    exit(err_num);
}

void destroy_ast(ast_node_ptr node){
    if (node == NULL || node == (ast_node_ptr) -1) return;
    while(node->parent != NULL){
        node = node->parent;
    }
    free_ast(node);
}

void free_ast(ast_node_ptr node){
    if (node == NULL || node == (ast_node_ptr) -1) return;
    if (node->children != NULL){
        for (int i = 0; node->children[i] != NULL; i++){
            free_ast(node->children[i]);
        }
        free(node->children);
    }
    free(node);
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





