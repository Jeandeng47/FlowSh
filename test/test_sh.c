#include "sh.h"
#include <stdio.h>


// ---------- Test function prototypes ----------
// test tokenize
int test_empty_tokens(void);
int test_leading_whitespace_tokens(void);
int test_multiple_words_tokens(void);
int test_pipe_and_redirect_tokens(void);
int test_complex_tokens(void);

// test parse_exec

int test_redirect_in(void);

int test_exec_no_args(void);
int test_exec_single_args(void);
int test_exec_multiple_args(void);
int test_exec_stop_at_pipe(void);

int test_pipe_no_pipe(void);
int test_pipe_simple(void);
int test_pipe_multiple(void);

int test_list_single(void);
int test_list_multiple(void);
int test_list_pipe(void);



// ---------- Test names and function pointers ----------
const char *test_names[] = {

    "test_redirect_in",

    "test_exec_no_args",
    "test_exec_single_args",
    "test_exec_multiple_args",
    "test_exec_stop_at_pipe",

    "test_pipe_no_pipe",
    "test_pipe_simple",
    "test_pipe_multiple",

    "test_list_single",
    "test_list_multiple",
    "test_list_pipe",

};

typedef int (*test_func)(void);
test_func tests[] = {
    test_redirect_in,
    
    test_exec_no_args,
    test_exec_single_args,
    test_exec_multiple_args,
    test_exec_stop_at_pipe,

    test_pipe_no_pipe,
    test_pipe_simple,
    test_pipe_multiple,

    test_list_single,
    test_list_multiple,
    test_list_pipe,
    
};

int main(void) {
    int num_tests = sizeof(tests) / sizeof(test_func);
    int passed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        if (tests[i]() == 0) {
            passed++;
            printf("Test %s passed\n", test_names[i]);
        } else {
            printf("Test %s failed\n", test_names[i]);
        }
    }
    printf("%d out of %d tests passed\n", passed, num_tests);
    return (passed == num_tests ? 0 : 1);
}