# define MAX_TOKENS 128

// Define types of commands
typedef enum {
    CMD_LIST,
    CMD_PIPE,
    CMD_REDIRCT,
    CMD_EXEC,
    CMD_BACKGROUND,
} CmdType;

// General command
struct cmd {
    CmdType type;
};

// List command: a list of commands executed sequentially
// Example: ls -l; pwd; echo "Hello"
struct cmd_list {
    CmdType type;
    struct cmd *left;
    struct cmd *right;
};

// Pipe command: a command that takes the output of one command 
// and uses it as input for another
// Example: cat file.txt | grep "pattern" | wc -l
struct cmd_pipe {
    CmdType type;
    struct cmd *left;
    struct cmd *right;
};

// Execute command: a command that executes a program with arguments
struct cmd_exec {
    CmdType type;
    char **agrs;
};

// Redirect command: a command that redirects input or output
// Example: cat file.txt > output.txt < input.txt