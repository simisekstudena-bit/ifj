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

// Initializes linked list
void list_init(list_t *l) {
    l->first = NULL;
    l->last = NULL;
}

// Inserts element to the end of the list
// Allocates space for node and instruction (string)
void list_append(list_t *l, const char *text) {
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

    if (l->first == NULL) {
        l->first = new_node;
        l->last = new_node;
    } else {
        l->last->next = new_node;
        l->last = new_node;
    }
}

// Frees every node and its string
void list_free(list_t *l) {
    while (l->first != NULL) {
        node_t *current = l->first;
        l->first = l->first->next;
        free(current->instruction);
        free(current);
    }
    l->last = NULL;
}

/* TODO:
    - implement functions for generator
*/

void gen_init(void) {

}

void gen_print(FILE *out) {
    (void)out;
}

void gen_free(void) {

}
