/*
Projekt: Implementace prekladace
Author: Patrik Kadleček xkadlep01

*/

#include "generator.h"
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
        exit(99);
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

// Formats instruction before appending to the code list
// ("PUSHS int@%d", 42) ==> PUSHS int@42
static void add_instruction(const char *format, ...) {
    va_list args;
    va_start(args, format);

    va_list args2;
    va_copy(args2, args);

    // Measures length
    int len = vsnprintf(NULL, 0, format, args);
    if (len < 0) {
        fprintf(stderr, "Failed to load instruction length\n");
        exit(99);
    }

    // Creates a buffer for instruction
    char *buf = malloc(len + 1);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(99);
    }

    // Writes formatted string into buffer
    vsnprintf(buf, len + 1, format, args2);

    va_end(args2);
    va_end(args);

    list_append(&code, buf);
}
// Converts string to final string literal format
// Formats characters 0-32, '#', '\' to \xyz format
// Returns allocated string (caller will free it)
static char *format_string_literal(const char *string) {
    size_t len = strlen(string);
    char *formatted = malloc(4*len+1);
    if(formatted == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(99);
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
