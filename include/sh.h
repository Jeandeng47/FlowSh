#ifndef SH_H
#define SH_H

# define MAX_TOKENS 128
# define MAX_ARGS 32

// Define types of commands
typedef enum {
    CMD_LIST,
    CMD_PIPE,
    CMD_REDIRCT,
    CMD_EXEC,
    CMD_BACKGROUND,
} cmd_type_t;

// General command
struct cmd {
    cmd_type_t type;
};

// List command: a list of commands executed sequentially
// Example: ls -l; pwd; echo "Hello"
struct cmd_list {
    cmd_type_t type;
    struct cmd *left;
    struct cmd *right;
};

// Pipe command: a command that takes the output of one command 
// and uses it as input for another
// Example: cat file.txt | grep "pattern" | wc -l
struct cmd_pipe {
    cmd_type_t type;
    struct cmd *left;
    struct cmd *right;
};

// Execute command: a command that executes a program with arguments
// Exampe: cat file.txt
struct cmd_exec {
    cmd_type_t type;
    char *args[MAX_ARGS];
};

// Redirect command: a command that redirects input or output
// Example: cat file.txt > output.txtt
struct cmd_redirect {
    cmd_type_t type;
    struct cmd *subcmd;
    char *file;
    int mode;
    int fd;
};

// Background command: a command that runs in the background
// Example: sleep 10 &
struct cmd_background {
    cmd_type_t type;
    struct cmd *subcmd;
};

// A generic command struct for testing
typedef struct exp_cmd {
    cmd_type_t type;
    // for exec
    char *argv[MAX_ARGS + 1];

    // for redirect
    char *file;
    int mode;
    int fd;

    // for pipe, list, redirect (sub_cmd in left), background (sub_cmd in left)
    struct exp_cmd *left;
    struct exp_cmd *right;

} exp_cmd_t;

// Function that tokenize
int tokenize(char *s, char* tokens[]);

struct cmd* new_listcmd(struct cmd *left, struct cmd *right);
struct cmd* new_pipecmd(struct cmd *left, struct cmd *right);
struct cmd* new_redircmd(struct cmd *subcmd, char *file, int mode, int fd);
struct cmd* new_execmd(char **argv);
struct cmd* new_backgroundcmd(struct cmd *subcmd);

struct cmd *parse_exec(char *tokens[], int *pos);
struct cmd *parse_pipe(char *tokens[], int *pos);
struct cmd *parse_line(char *tokens[], int *pos);


#endif