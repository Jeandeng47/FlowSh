#include "sh.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ---------- Test helpers  ----------

// The smallest cmd that could not be further parsed
static int compare_exec(struct cmd *actual, exp_cmd_t *exp) {
    // compare args
    struct cmd_exec *ce = (struct cmd_exec*)actual;
    for (int i = 0; exp->argv[i] != NULL; i++) {
        if (!ce->args[i] || strcmp(ce->args[i], exp->argv[i]) != 0) {
            printf(
                "EXEC mismatch: argv[%d] expected '%s' but got '%s'\n",
                i,
                exp->argv[i],
                ce->args[i] ? ce->args[i] : "NULL");
            return -1;
        }
    }
    return 0;
}

// A general function for comparing actual cmd and expected cmd
static int compare_cmd(struct cmd *actual, exp_cmd_t *exp) {
    if (!actual) {
        printf("Actual cmd is NULL, expected type %d\n", exp->type);
        return -1;
    }
    // check type
    if (actual->type != exp->type) {
        printf("Type mismatch: expected %d but got %d\n", exp->type, actual->type);
        return -1;
    }

    switch(exp->type) {
        case CMD_EXEC:
            return compare_exec(actual, exp);
        case CMD_PIPE: {
            struct cmd_pipe *p = (struct cmd_pipe*) actual;
            // recursively check left and right
            if (compare_cmd(p->left, exp->left) < 0) return -1;
            if (compare_cmd(p->right, exp->right) < 0) return -1;
            return 0;
        }
        case CMD_LIST: {
            struct cmd_list *l = (struct cmd_list*) actual;
            // recursively check left and right
            if (compare_cmd(l->left, exp->left) < 0) return -1;
            if (compare_cmd(l->right, exp->right) < 0) return -1;
            return 0;
        }

        case CMD_REDIRCT: {
            struct cmd_redirect *r = (struct cmd_redirect*) actual;
            if (compare_cmd(r->subcmd, exp->left) < 0) return -1;
            if (strcmp(r->file, exp->file) != 0) return -1;
            if (r->mode != exp->mode || r->fd != exp->fd) return -1;
            return 0;
        }

        case CMD_BACKGROUND: {
            struct cmd_background *b = (struct cmd_background*) actual;
            if (compare_cmd(b->subcmd, exp->left) < 0) return -1;
            return 0;
        }

        default:
            printf("Unsupported CmdType %d\n", exp->type);
            return -1;
            
    }
}

// ---------- Test implementations using compare_cmd ----------

int test_redirect_in(void) {
    char *t[] = { "cmd", "<", "input.txt", NULL};

    int pos = 0;
    struct cmd *c = parse_exec(t, &pos);

    exp_cmd_t sub = { .type = CMD_EXEC, .left=NULL, .right=NULL };
    sub.argv[0] = "cmd"; sub.argv[1] = NULL;
    exp_cmd_t exp = { .type = CMD_REDIRCT, .left = &sub, .right = NULL};
    exp.file = "input.txt";
    exp.fd = 0;
    exp.mode = O_RDONLY;

    int res = (compare_cmd(c, &exp) == 0 && pos == 3);
    free(c);

    return res ? 0 : -1;
}

int test_exec_no_args(void) {
    char *t[] = { NULL };

    exp_cmd_t exp = { .type = CMD_EXEC };
    exp.argv[0] = NULL;

    int pos = 0;
    struct cmd *c = parse_exec(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 0);

    free(c);
    return res? 0 : -1;
}

int test_exec_single_args(void) {
    char *t[] = { "ls", NULL };

    exp_cmd_t exp = { .type = CMD_EXEC };
    exp.argv[0] = "ls";
    exp.argv[1] = NULL;

    int pos = 0;
    struct cmd *c = parse_exec(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 1);

    free(c);
    return res? 0 : -1;
}

int test_exec_multiple_args(void) {
    char *t[] = { "echo","hello","world",NULL };

    exp_cmd_t exp = { .type = CMD_EXEC };
    exp.argv[0] = "echo";
    exp.argv[1] = "hello";
    exp.argv[2] = "world";
    exp.argv[3] = NULL;

    int pos = 0;
    struct cmd *c = parse_exec(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 3);

    free(c);
    return res? 0 : -1;
}

int test_exec_stop_at_pipe(void) {
    char *t[] = { "cmd","|","more",NULL };

    exp_cmd_t exp = { .type = CMD_EXEC };

    exp.argv[0] = "cmd";
    exp.argv[1] = NULL;

    int pos = 0;
    struct cmd *c = parse_exec(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 1);

    free(c);
    return res? 0: -1;
}

int test_pipe_no_pipe(void) {
    char *t[] = { "pwd", NULL };

    exp_cmd_t exp = { .type = CMD_EXEC };
    exp.argv[0] = "pwd";
    exp.argv[1] = NULL;

    int pos = 0;
    struct cmd *c = parse_pipe(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 1);
    free(c);
    return res? 0: -1;
}


int test_pipe_simple(void) {
    char *t[] = { "ls","|","grep","foo",NULL };

    exp_cmd_t left = { .type = CMD_EXEC };
    left.argv[0] = "ls";
    left.argv[1] = NULL;

    exp_cmd_t right = { .type = CMD_EXEC };
    right.argv[0] = "grep";
    right.argv[1] = "foo";
    right.argv[2] = NULL;

    exp_cmd_t exp = { .type = CMD_PIPE, .left = &left, .right = &right };

    int pos = 0;
    struct cmd *c = parse_pipe(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 4);

    free(c);
    return res ? 0 : -1;
}


int test_pipe_multiple(void) {
    char *t[] = { "a", "|", "b", "|", "c", NULL };
    // expected AST: pipe( pipe(a,b), c )
    exp_cmd_t e1 = { .type = CMD_EXEC, .argv = { "a", NULL } };
    exp_cmd_t e2 = { .type = CMD_EXEC, .argv = { "b", NULL } };
    exp_cmd_t e3 = { .type = CMD_EXEC, .argv = { "c", NULL } };

    exp_cmd_t mid = { .type = CMD_PIPE, .left = &e1, .right = &e2 };
    exp_cmd_t exp = { .type = CMD_PIPE, .left = &mid, .right = &e3 };

    int pos = 0;
    struct cmd *c = parse_pipe(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 5);

    free(c);
    return res ? 0 : -1;
}

int test_list_single(void) {
    char *t[] = { "ls", ";", "pwd", NULL };
    exp_cmd_t e1 = { .type = CMD_EXEC, .argv = {"ls", NULL} };
    exp_cmd_t e2 = { .type = CMD_EXEC, .argv = {"pwd", NULL} };
    exp_cmd_t exp = { .type = CMD_LIST, .left = &e1, .right = &e2 };

    int pos = 0;
    struct cmd *c = parse_line(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 3);

    free(c);
    return res? 0 : -1;
}

int test_list_multiple(void) {
    char *t[] = { "cmd1", ";", "cmd2", ";", "cmd3", NULL };
    exp_cmd_t e1 = { .type = CMD_EXEC, .argv = { "cmd1", NULL } };
    exp_cmd_t e2 = { .type = CMD_EXEC, .argv = { "cmd2", NULL } };
    exp_cmd_t e3 = { .type = CMD_EXEC, .argv = { "cmd3", NULL } };
    exp_cmd_t l12 = { .type = CMD_LIST, .left = &e1, .right = &e2 };
    exp_cmd_t exp = { .type = CMD_LIST, .left = &l12, .right = &e3 };

    int pos = 0;
    struct cmd *c = parse_line(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 5);
    free(c);
    return res ? 0 : -1;
}

int test_list_pipe(void) {
    char *t[] = { "ls", "|", "grep", "foo", ";", "echo", "done", NULL };
    exp_cmd_t e_ls    = { .type = CMD_EXEC, .argv = { "ls", NULL } };
    exp_cmd_t e_gr    = { .type = CMD_EXEC, .argv = { "grep", "foo", NULL } };
    exp_cmd_t pipe1   = { .type = CMD_PIPE, .left = &e_ls, .right = &e_gr };
    exp_cmd_t e_echo  = { .type = CMD_EXEC, .argv = { "echo", "done", NULL } };
    exp_cmd_t exp     = { .type = CMD_LIST, .left = &pipe1, .right = &e_echo };

    int pos = 0;
    struct cmd *c = parse_line(t, &pos);
    int res = (compare_cmd(c, &exp) == 0 && pos == 7);
    free(c);
    return res ? 0 : -1;
}

