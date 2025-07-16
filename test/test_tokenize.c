#include "sh.h"
#include <stdio.h>
#include <string.h>

// Helper function to print tokens for debugging
void print_tokens(char *tokens[]) {
    for (int i = 0; tokens[i] != NULL; i++) {
        printf("[DBG] Token %d: '%s'\n", i, tokens[i]);
    }
}

// Function to test the tokenize function
static int test_tokenize(char *input, char *expected[], int expected_count) {
    char buf[4096]; 
    char *tokens[MAX_TOKENS];
    memset(buf, 0, sizeof(buf));
    memset(tokens, 0, sizeof(tokens));

    strncpy(buf, input, sizeof(buf)); 
    buf[sizeof(buf) - 1] = '\0';

    int ntokens = tokenize(buf, tokens);
    
    // check number of tokens
    if (ntokens != expected_count) {
        printf("Expected %d tokens, got %d\n", expected_count, ntokens);
        return -1;
    }

    // check each token
    for (int i = 0; i < ntokens; i++) {
        if (strcmp(tokens[i], expected[i]) != 0) {
            printf("Token %d: expected '%s', got '%s'\n", i, expected[i], tokens[i]);
            return -1;
        }
    }
    return 0;
}

int test_empty_tokens() {
    char *input = "";
    char *expected[] = { NULL };
    return test_tokenize(input, expected, 0);
}

int test_leading_whitespace_tokens() {
    char *input = "   > out.txt";
    char *expected[] = { ">", "out.txt", NULL };
    return test_tokenize(input, expected, 2);
}

int test_multiple_words_tokens() {
    char *input = "cat tokens";
    char *expected[] = { "cat", "tokens", NULL };
    return test_tokenize(input, expected, 2);
}


int test_pipe_and_redirect_tokens() {
    char *input = "cat file.txt | grep 'pattern' > output.txt";
    char *expected[] = { "cat", "file.txt", "|", "grep", "'pattern'", ">", "output.txt", NULL };
    return test_tokenize(input, expected, 7);
}

int test_complex_tokens() {
    char *input = "ls -l | grep 'test' > output.txt < input.txt &";
    char *expected[] = { "ls", "-l", "|", "grep", "'test'", ">", "output.txt", "<", "input.txt", "&", NULL };
    return test_tokenize(input, expected, 10);
}

