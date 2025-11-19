#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "scanner.h"
#include "ast.h"
#include "generator.h"
#include "symtable.h"


//REMOVE, FOR TESTING PURPOSES ONLY

// --- BST Visualization Helpers ---

void print_bst_branch(bst_node_ptr node, char *prefix, int is_left) {
    if (node == NULL) return;

    printf("%s", prefix);
    printf("%s", is_left ? "├──" : "└──");

    // Print the Node Content (Key and Name)
    // Adjust formatting based on your needs
    printf(" [Key: %u] Var: '%s'", node->key, node->content.name);
    
    // Show type if available (assuming you have a way to print enum types)
    // printf(" Type: %d", node->content.type); 
    
    printf("\n");

    // Prepare prefix for children
    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_left ? "│   " : "    ");

    // Recurse
    // We check if children exist to avoid printing empty branches unnecessarily
    // But if you want to see NULLs, you can remove the checks.
    if (node->left || node->right) {
        if (node->left) {
            print_bst_branch(node->left, new_prefix, 1); // 1 = is_left (not last)
        } else {
            // Print a placeholder if left is missing but right exists
            printf("%s├── (null)\n", new_prefix);
        }

        if (node->right) {
            print_bst_branch(node->right, new_prefix, 0); // 0 = is_right (last)
        } else {
            printf("%s└── (null)\n", new_prefix);
        }
    }
}

void visualize_bst_scope(bst_scope_ptr scope, int depth) {
    if (scope == NULL) return;

    // Indent based on scope depth
    for (int i = 0; i < depth; i++) printf("    ");
    
    printf("📦 SCOPE (Depth %d) ", depth);
    if (scope->parent == NULL) printf("[GLOBAL]");
    printf("\n");

    // 1. Print the BST contents of this scope
    if (scope->tree == NULL) {
        for (int i = 0; i < depth; i++) printf("    ");
        printf("    (Empty Symbol Table)\n");
    } else {
        // Create an initial indentation string for the tree
        char tree_prefix[256] = "";
        for (int i = 0; i < depth; i++) strcat(tree_prefix, "    ");
        
        // Print the root of this scope's BST
        printf("%s└── [ROOT] '%s' (%u)\n", tree_prefix, scope->tree->content.name, scope->tree->key);
        
        // Prepare prefix for children
        char child_prefix[256];
        //snprintf(child_prefix, sizeof(child_prefix), "%s    ", tree_prefix);

        // Trigger recursive printer
        if (scope->tree->left) print_bst_branch(scope->tree->left, child_prefix, 1);
        if (scope->tree->right) print_bst_branch(scope->tree->right, child_prefix, 0);
    }

    // 2. Recursively print child scopes
    for (int i = 0; i < scope->n_of_children; i++) {
        visualize_bst_scope(scope->child[i], depth + 1);
    }
}

// --- Entry Point ---
void visualize_symbol_table(bst_scope_ptr current_scope) {
    printf("\n📚 SYMBOL TABLE VISUALIZATION\n");
    printf("================================\n");
    if (current_scope == NULL) {
        printf("Symbol Table is NULL.\n");
        return;
    }
    
    // Walk up to the actual root
    bst_scope_ptr root = current_scope;
    while (root->parent != NULL) {
        root = root->parent;
    }
    
    // Now visualize from the actual root
    visualize_bst_scope(root, 0);
    printf("================================\n\n");
}

// Helper to safely print null strings
const char* safe_str(char* s) {
    return s ? s : "NULL_STR";
}

void register_parameters(ast_node_ptr param_node, bst_scope_ptr scope) {
  //  printf("[DEBUG] register_parameters: Entering. Node=%p, Scope=%p\n", (void*)param_node, (void*)scope);
    fflush(stdout);

    if (param_node == NULL) {
     //   printf("[DEBUG] register_parameters: param_node is NULL. Returning.\n");
        fflush(stdout);
        return;
    }
    
  //  printf("[DEBUG] register_parameters: n_of_children=%d\n", param_node->n_of_children);
    fflush(stdout);

    for (int i = 0; i < param_node->n_of_children; i++) {
        ast_node_ptr arg = param_node->children[i];
      //  printf("[DEBUG] register_parameters: Processing child %d at %p\n", i, (void*)arg);
        fflush(stdout);

        if (arg == NULL) {
         //   printf("[DEBUG] FATAL: Parameter child is NULL!\n");
            fflush(stdout);
            continue;
        }

        if (arg->token.type == TT_IDENTIFIER) {
          //  printf("[DEBUG] register_parameters: Declaring param '%s'\n", safe_str(arg->token.lexeme));
            fflush(stdout);

            int res = bst_declare_variable(scope, arg->token.lexeme);
            if (res == 1) {
              //  fprintf(stderr, "Semantic Error: Duplicate parameter name '%s'\n", safe_str(arg->token.lexeme));
                exit(3);
            }
        } else {
         //   printf("[DEBUG] register_parameters: Child is not TT_IDENTIFIER. Type=%d\n", arg->token.type);
            fflush(stdout);
        }
    }
 //   printf("[DEBUG] register_parameters: Exiting.\n");
    fflush(stdout);
}

void analyze_node(ast_node_ptr node, bst_scope_ptr current_scope) {
    if (node == NULL) {
        // This is common in recursion, not necessarily an error
        return;
    }

 //   printf("[DEBUG] analyze_node: Visiting Node=%p, Type=%d, Lexeme='%s', n_children=%d\n", 
  //         (void*)node, node->token.type, safe_str(node->token.lexeme), node->n_of_children);
    fflush(stdout);

    // 1. Variable Declaration
    if (node->token.type == TT_KEYWORD_VAR) {
     //   printf("[DEBUG] analyze_node: Found VAR declaration.\n");
        fflush(stdout);
        
        // --- NEW LOGIC START ---
        char *var_name = NULL;

        if (node->n_of_children > 0 && node->children[0] != NULL) {
            // Case 1: Variable name is the first child (e.g., in a more complex AST)
            var_name = node->children[0]->token.lexeme;
         //   printf("[DEBUG] analyze_node: VAR name found in child[0].\n");
        } else if (node->token.lexeme != NULL) {
            // Case 2 (Your current AST): Variable name is directly on the VAR token
            var_name = node->token.lexeme;
         //   printf("[DEBUG] analyze_node: VAR name found in own lexeme.\n");
        }

        if (var_name != NULL) {
            // New Debug Print: Capture the hash key before declaration
            //unsigned int decl_hash = get_hash(var_name);
         //   printf("[DEBUG] analyze_node: Declaring '%s', HASH KEY: %u\n", safe_str(var_name), decl_hash);
            fflush(stdout);

            //int result =
            bst_declare_variable(current_scope, var_name); // Note: bst_declare_variable must use var_name to calculate the same hash!
        //    if (result == 1) {
        //        fprintf(stderr, "Semantic Error: Variable '%s' already declared.\n", var_name);
        //        exit(3);
        //    }
        } else {
          //  printf("[DEBUG] analyze_node: VAR node contains no recognizable name. Skipping.\n");
        }
        //visualize_symbol_table(current_scope); // DEBUG Visualize after declaration
        // --- NEW LOGIC END ---
    }
    
    // 2. Variable Usage
    else if (node->token.type == TT_IDENTIFIER && node->node_type == NT_ID) {
        //char *var_name = node->token.lexeme;
        
        // New Debug Print: Capture the hash key before search
        //unsigned int usage_hash = get_hash(var_name);
      //  printf("[DEBUG] analyze_node: Checking usage of '%s', HASH KEY: %u\n", safe_str(var_name), usage_hash);
        fflush(stdout);
        
    //    bst_node_content_t data = bst_search_scope(current_scope, usage_hash); // Use the captured hash
    //    if (data.type == TT_ERROR) {
    //         fprintf(stdout, "Semantic Error: Variable '%s' used but not defined.\n", var_name);
    //         exit(3);
    //    }
    }   

    // 3. Scope creation
    bool creates_scope = (node->token.type == TT_KEYWORD_IF || 
                          node->token.type == TT_KEYWORD_WHILE ||
                          node->token.type == TT_KEYWORD_ELSE);

    if (creates_scope) {
     //   printf("[DEBUG] analyze_node: Entering new scope (IF/WHILE/ELSE)\n");
        fflush(stdout);
        bst_increase_scope(&current_scope);
    }

    // 4. Recursion
    if (node->n_of_children > 0 && node->children != NULL) {
        for (int i = 0; i < node->n_of_children; i++) {
            // printf("[DEBUG] analyze_node: Recursing to child %d/%d\n", i+1, node->n_of_children); // Optional: Uncomment for very verbose logs
            analyze_node(node->children[i], current_scope);
        }
    }

    // 5. Exit Scope
    if (creates_scope) {
     //   printf("[DEBUG] analyze_node: Exiting scope (IF/WHILE/ELSE)\n");
        fflush(stdout);
        bst_decrease_scope(&current_scope);
    }
}

void run_semantic_analysis(ast_node_ptr root, bst_scope_ptr current_scope) {
   // printf("\n[DEBUG] START Semantic Analysis. Root=%p, Scope=%p\n", (void*)root, (void*)current_scope);
    fflush(stdout);

    if (root == NULL) return;

    // --- Pass 1: Register Function Names ---
  //  printf("[DEBUG] Pass 1: Registering Functions...\n");
    fflush(stdout);
    for (int i = 0; i < root->n_of_children; i++) {
        ast_node_ptr child = root->children[i];
        
        if (child == NULL) {
         //   printf("[DEBUG] Pass 1: Child %d is NULL. Skipping.\n", i);
            fflush(stdout);
            continue;
        }

        if (child->node_type == NT_FUNC_DECL) {
            char *func_name = child->token.lexeme;
          //  printf("[DEBUG] Pass 1: Found Function '%s'\n", safe_str(func_name));
            fflush(stdout);

            // 🛑 CRASH POINT 1: Searching for Redefinition
         //   printf("[DEBUG] Pass 1: Attempting bst_search_scope for '%s'...\n", safe_str(func_name));
            fflush(stdout); 

        //    if(bst_search_scope(current_scope, get_hash(func_name)).type != TT_ERROR) {
          //      fprintf(stderr, "Semantic Error: Function '%s' redefinition.\n", func_name);
          //      exit(3);
          //  }
         //   printf("[DEBUG] Pass 1: Search successful.\n");
            fflush(stdout);
            
            // 🛑 CRASH POINT 2: Declaring the Variable/Function
         //   printf("[DEBUG] Pass 1: Attempting bst_declare_variable for '%s'...\n", safe_str(func_name));
            fflush(stdout); 
            bst_declare_variable(current_scope, func_name); 
         //   printf("[DEBUG] Pass 1: Declaration successful.\n");
            fflush(stdout);
        }
    }

    // --- PASS 2: Check Function Bodies ---
    for (int i = 0; i < root->n_of_children; i++) {
        ast_node_ptr child = root->children[i];
        
        if (child->node_type == NT_FUNC_DECL) {
            
            // 1. ENTER SCOPE
            bst_increase_scope(&current_scope);
         //   printf("[DEBUG] Entered scope for function: %s\n", safe_str(child->token.lexeme));
            
            // 2. REGISTER PARAMETERS (This is crucial, you already implemented this)
            if (child->n_of_children > 0 && child->children[0]->node_type == NT_PARAM) {
                register_parameters(child->children[0], current_scope);
            }

            // 3. SUB-PASS 1: REGISTER LOCAL DECLARATIONS (Fixing your error!)
        //    printf("[DEBUG] Sub-Pass 1: Registering local variables...\n");
            for(int j = 0; j < child->n_of_children; j++) {
                ast_node_ptr body_node = child->children[j];
                
                // Only process the declaration node itself (e.g., 'var a')
                if (body_node != NULL && body_node->token.type == TT_KEYWORD_VAR) {
                    // Call analyze_node ONLY to handle the TT_KEYWORD_VAR block
                    // analyze_node should ONLY register the variable and NOT recurse here.
                    analyze_node(body_node, current_scope); 
                }
            }
            
            // 4. SUB-PASS 2: CHECK USAGE & TRAVERSE SCOPES
        //    printf("[DEBUG] Sub-Pass 2: Checking usage and scope traversal...\n");
            for(int j = 0; j < child->n_of_children; j++) {
                ast_node_ptr body_node = child->children[j];

                // Skip parameter nodes and raw declaration nodes, as they were handled.
                if (body_node != NULL && 
                    body_node->node_type != NT_PARAM && 
                    body_node->token.type != TT_KEYWORD_VAR) 
                {
                    // This is where assignments, calls, if/while blocks are processed.
                    // The usage checks and recursive scope logic happen inside analyze_node.
                    analyze_node(body_node, current_scope);
                }
            }
            
            // 5. EXIT SCOPE
         //   printf("[DEBUG] Exited scope for function: %s\n", safe_str(child->token.lexeme));
            bst_decrease_scope(&current_scope);
        }
    }
}


// --- Recursive visualizer ---
void print_ast_branch(const ast_node_ptr node, const char *prefix, int is_last) {
    if (!node) {
        printf("%s%s NULL NODE\n", prefix, is_last ? "└── " : "├── ");
        return;
    }

    // Check if pointer looks valid (not a common invalid value)
    //if ((unsigned long)node < 0x1000) {
      //  printf("%s%s INVALID POINTER: %p\n", prefix, is_last ? "└── " : "├── ", (void*)node);
        //return;
   // }

    printf("%s%s ", prefix, is_last ? "└── " : "├── ");
    fflush(stdout);

    // Try to access token.type
    token_type_t type;
    type = node->token.type;
    printf("%s", token_type_to_string(type));
    fflush(stdout);

    // Try to access lexeme
    if (node->token.lexeme) {
        printf(" ('%s')", node->token.lexeme);
    }
    fflush(stdout);

    // Try to access n_of_children
    int n_children = node->n_of_children;
    printf(" [children: %d]", n_children);
    fflush(stdout);

    printf("\n");
    fflush(stdout);

    // Prepare new prefix
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    // Check children pointer
    if (n_children > 0 && node->children == NULL) {
        printf("%sERROR: n_of_children=%d but children is NULL\n", new_prefix, n_children);
        return;
    }

    // Recursively print children
    for (int i = 0; i < n_children; i++) {
        printf("%sDEBUG: Child %d/%d at %p\n", new_prefix, i+1, n_children, (void*)node->children[i]);
        fflush(stdout);
        print_ast_branch(node->children[i], new_prefix, i == n_children - 1);
    }
}

// --- Entry function ---
void visualize_ast(const ast_node_ptr root) {
    printf("🌳 Abstract Syntax Tree\n");
    fflush(stdout);
    if (root == NULL) {
        printf("ROOT IS NULL!\n");
        return;
    }
    printf("Root at %p\n", (void*)root);
    fflush(stdout);
    print_ast_branch(root, "", 1);
    printf("\nVisualization complete!\n");
    fflush(stdout);
}

int main(void) {

    /*
    token_t tok;
    while ((tok = get_token()).type != TT_EOF) {
        printf("TOKEN: type=%d, lexeme=\"%s\"\n", tok.type, tok.lexeme);
        free(tok.lexeme); // FREE THE MEMORY!
    }
    printf("TOKEN: type=%d, lexeme=\"%s\"\n", tok.type, tok.lexeme);
     */

    
    ast_node_ptr ast_root = create_ast();


    //visualize_ast(ast_root);

    bst_scope_ptr current_scope = malloc(sizeof(bst_scope_t));
    if (current_scope == NULL) {
        return 99;;
    }

    current_scope->tree = NULL;
    current_scope->parent = NULL;
    current_scope->child = NULL;
    current_scope->n_of_children = 0;

    run_semantic_analysis(ast_root, current_scope);
    //visualize_symbol_table(current_scope);

    generate_code(ast_root, current_scope);
    fflush(stdout);
    //destroy_ast(ast_root);
    free_ast(&ast_root);
    
    //visualize_ast(ast_root);
    fflush(stdout);
    //printf(ast_root == NULL ? "AST successfully freed.\n" : "AST freeing failed!\n");
    //ast_print_token(ast_root->token);
    //destroy_ast(ast_root); Doesnt work
    bst_destroy_symbol_table(current_scope);
    //printf("Symbol Table successfully freed.\n");


    return 0;
}





