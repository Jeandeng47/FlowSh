
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"

static bool is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static bool is_symbol(char c) {
    return strchr("<>|&;", c) != NULL;
}

static int get_token(char **pa, char *line_end, char **tok_start, char **tok_end) {
    char *p = *pa; // pa = address of the scanning pointer, *pa = p = address of the current character

    while (p < line_end && is_space(*p)) p++; // *p = current character
    *tok_start = p; // *tok_start = address of the first non-whitespace character
    
    if (p >= line_end || *p == '\0') return 0;
    int kind = *p;

    if (is_symbol(*p)) {
        // Single special symbol
        p++;
    } else {
        // Normal word
        while (p < line_end && !is_space(*p) && !is_symbol(*p)) { // stop at whitespace or special symbol
            p++;
        }
    }
    *tok_end = p; // *tok_end = address of token end

    while (p < line_end && is_space(*p)) p++; // skip trailing whitespace
    *pa = p; // update the scanning pointer

    return kind;
}


// Function to turn input line into tokens
int tokenize(char *line, char* tokens[]) {
    char *ptr = line;
    char *line_end = line + strlen(line);
    int ntokens = 0;

    while (ntokens < MAX_TOKENS) {
        char *start, *end;
        int kind = get_token(&ptr, line_end, &start, &end);
        if (kind == 0) break;
        *end = '\0';
        tokens[ntokens++] = start;
    }
    tokens[ntokens] = NULL;
    return ntokens;
}


// Function to parse the command string into a command structure
// struct cmd *parse_cmd(const char *buffer) {
//     char *buf = strdup(buffer); // mutable copy

//     char *tokens[MAX_TOKENS];
//     int ntokens = tokenize(buf, tokens);
//     return NULL;
// }

// Function to read a command from standard input
int read_cmd(char *buffer, int bufsize) {
    printf("flowsh> ");

    if (!fgets(buffer, bufsize, stdin)) {
        return 0;
    }
    
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return 1;
}


// int main(void) {
//     char buffer[1024];
//     struct cmd *command = NULL;
//     // 1. Print prompt and get input
//     while (read_cmd(buffer, sizeof(buffer)) > 0) {
//         // 2. Parse the input into a command structure
//         command = parse_cmd(buffer);
//         // 3. Execute the command
//     }
// }