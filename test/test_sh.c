#include "parser.h"
#include <stdio.h>

typedef int (*test_func)(void);

int test_empty_tokens(void);
int test_leading_whitespace_tokens(void);
int test_multiple_words_tokens(void);
int test_pipe_and_redirect_tokens(void);
int test_complex_tokens(void);

test_func tests[] = {
    test_empty_tokens,
    test_leading_whitespace_tokens,
    test_multiple_words_tokens,
    test_pipe_and_redirect_tokens,
    test_complex_tokens
};

int main(void) {
    int num_tests = sizeof(tests) / sizeof(test_func);
    int passed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        if (tests[i]() == 0) {
            passed++;
            printf("Test %d passed\n", i + 1);
        } else {
            printf("Test %d failed\n", i + 1);
        }
    }
    printf("%d out of %d tests passed\n", passed, num_tests);
    return (passed == num_tests ? 0 : 1);
}