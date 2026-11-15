/*
Projekt: Implementace prekladace
Author: Patrik Kadleček xkadlep01

*/

#include "generator.h"
#include "errors.h"
#include <string.h>
#include <stdarg.h>

// Struct for keeping instructions
// One node == one instruction
typedef struct node {
    char *instruction;
    struct node *next;
} node_t;

// Linked list of instructions
typedef struct list {
    node_t *first;
    node_t *last; // keeping last so we can append fast - O(1)
} list_t;

static list_t code;

// Initializes linked list
static void list_init(list_t *l) {
    l->first = NULL;
    l->last = NULL;
}

// Inserts element to the end of the list
// Allocates space for node and gets ownership of formatted instruction
static void list_append(list_t *l, char *text) {
    node_t *new_node = malloc(sizeof(node_t));

    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        free(text);
        exit(INTERNAL_INTERPRET_ERR);
    }
    // Takes ownership of instruction
    new_node->instruction = text;
    new_node->next = NULL;

    // No elements
    if (l->first == NULL) {
        l->first = new_node;
        l->last = new_node;
    } else { // One or more elements
        l->last->next = new_node;
        l->last = new_node;
    }
}

// Frees every node and its string
static void list_free(list_t *l) {
    while (l->first != NULL) {
        node_t *current = l->first;
        l->first = l->first->next;
        free(current->instruction);
        free(current);
    }
    l->last = NULL;
}

// Formats instruction ("PUSHS int@%d", 42) ==> PUSHS int@42
// Returns formatted instruction
static char *vformat(const char *format, va_list args) {
    va_list args2;
    va_copy(args2, args);

    // Measures length
    int len = vsnprintf(NULL, 0, format, args);
    if (len < 0) {
        fprintf(stderr, "Failed to load instruction length\n");
        exit(INTERNAL_INTERPRET_ERR);
    }

    // Creates a buffer for instruction
    char *buf = malloc(len + 1);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(INTERNAL_INTERPRET_ERR);
    }

    // Writes formatted string into buffer
    vsnprintf(buf, len + 1, format, args2);
    va_end(args2);
    return buf;
}

// Helper function for format_literal
// Returns what vformat returned
static char *alloc_format(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *result = vformat(format, args);
    va_end(args);
    return result;
}

// Appends instruction to code list
static void add_instruction(const char *format, ...) {
    va_list args;
    va_start(args, format);

    list_append(&code, vformat(format, args));
    va_end(args);
}

// Converts string to final string literal format
// Formats characters 0-32, '#', '\' to \xyz format
// Returns allocated string (caller will free it)
static char *format_string_literal(const char *string) {
    size_t len = strlen(string);
    char *formatted = malloc(4*len+1);
    if(formatted == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(INTERNAL_INTERPRET_ERR);
    }
    int pos = 0;

    for(size_t i = 0; i < len; i++) {
        unsigned char ch = string[i];
        if(ch <= 32 || ch == '\\' || ch == '#') {
            sprintf(formatted + pos, "\\%03d", ch);
            pos += 4;
        } else {
            formatted[pos++] = ch;
        }
    }
    formatted[pos] = '\0';
    return formatted;
}

// Converts literal node to operand (e.g. "int@5")
// Returns allocated string (caller must free)
static char *format_literal(ASTNode *node) {
    switch (node->type) {
        case NODE_INT_LITERAL:
            return alloc_format("int@%d", node->int_value);
        case NODE_DOUBLE_LITERAL:
            return alloc_format("float@%a", node->double_value);
        case NODE_STRING_LITERAL: {
            char *formatted = format_string_literal(node->string_value);
            char *operand = alloc_format("string@%s", formatted);
            free(formatted);
            return operand;
        }
        case NODE_UNIT_LITERAL:
            return alloc_format("nil@nil");
        // not a literal (throw error)
        default:
            fprintf(stderr, "format_literal: not a literal node\n");
            exit(INTERNAL_INTERPRET_ERR);
    }
}

void gen_init(void) {
    list_init(&code);
    add_instruction(".IFJcode26");
}

void gen_print(FILE *out) {
    node_t *current = code.first;

    while (current != NULL) {
        fprintf(out, "%s\n", current->instruction);
        current = current->next;
    }
}

void gen_free(void) {
    list_free(&code);
}

void gen_program(ASTNode *root) {

    ASTNode *node = root->body;
    while(node != NULL) {
        if(node->is_main) {
            add_instruction("JUMP %s", node->name);
            break;
        }
        node = node->next;
    }
}
