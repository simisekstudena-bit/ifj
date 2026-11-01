/*
Projekt: Implementace prekladace
Author: Patrik Kadleček xkadlep01

*/

#include "generator.h"
#include <string.h>

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
// Allocates space for node and instruction (string)
static void list_append(list_t *l, const char *text) {
    node_t *new_node = malloc(sizeof(node_t));

    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(99);
    }

    char *instruction = malloc(strlen(text) + 1);
    if (instruction == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        free(new_node);
        exit(99);
    }
    strcpy(instruction, text);
    new_node->instruction = instruction;
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

void gen_init(void) {
    list_init(&code);
    list_append(&code, ".IFJcode26");
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
