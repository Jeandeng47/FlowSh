#include "sh.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// ---------- Construct commands ----------
struct cmd* new_listcmd(struct cmd *left, struct cmd *right) {
    struct cmd_list *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->type = CMD_LIST;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

struct cmd* new_pipecmd(struct cmd *left, struct cmd *right) {
    struct cmd_pipe *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->type = CMD_PIPE;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

struct cmd* new_redircmd(struct cmd *subcmd, char *file, int mode, int fd) {
    struct cmd_redirect *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->type = CMD_REDIRCT;
    cmd->subcmd = subcmd;
    cmd->file = file;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct cmd *)cmd;
}

struct cmd* new_execmd(char **args) {
    struct cmd_exec *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->type = CMD_EXEC;
    
    int argc = 0;
    while (args[argc] != NULL) argc++;

    for (int i = 0; i < argc && i < MAX_ARGS; i++) {
        cmd->args[i] = args[i]; // shallow copy
    }
    cmd->args[argc] = NULL; 

    return (struct cmd *)cmd;
}

struct cmd* new_backgroundcmd(struct cmd *subcmd) {
    struct cmd_background *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->type = CMD_BACKGROUND;
    cmd->subcmd = subcmd;
    return (struct cmd *)cmd;
}


// ---------- Parsing logic ----------

// Check if the character is a whitespace character
static bool is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

// Check if the character is a special symbol used in commands
static bool is_symbol(char c) {
    return strchr("<>|&;", c) != NULL;
}

// Function to get the next token from the input line
static int get_token(char **pa, char *line_end, char **tok_start, char **tok_end) {
    char *p = *pa; // pa = address of the scanning pointer, *pa = p = address of the current char

    while (p < line_end && is_space(*p)) p++; // *p = current char
    *tok_start = p; // *tok_start = address of the first non-whitespace char
    
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

    while (p < line_end && is_space(*p)) p++;
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
        *end = '\0'; // null terminate in-place
        tokens[ntokens++] = start;
    }
    tokens[ntokens] = NULL;
    return ntokens;
}

// Recursive descent parsing:
// -- parse_line: parse ;
// -- parse_pipe: parse |
// -- parse_exec: parse command execution

// Example: grep foo | sort ; echo done
// parse_line
//   -> parse_pipe
//       -> parse_exec (grep foo)
//   -> parse_pipe: "|" 
//       -> parse_exec (sort)
//   -> parse_pipe: return pipe command
// parse_line: ";"
//   -> parse_pipe
//       -> parse_exec (echo done)
// parse_line: return list command

struct cmd *parse_exec(char *tokens[], int *pos) {

    // collect argv until special char is met
    int start = *pos;
    while (tokens[*pos] 
        && strcmp(tokens[*pos], "|")!=0
        && strcmp(tokens[*pos], ";")!=0
        && strcmp(tokens[*pos], "<")!=0
        && strcmp(tokens[*pos], ">")!=0
        && strcmp(tokens[*pos], "&")!=0) {
        (*pos)++;
    }
    int argc = *pos - start;
    char *args[MAX_ARGS + 1];
    for (int i = 0; i < argc; i++) {
        args[i] = tokens[start + i];
    }
    args[argc] = NULL;

    // construct new cmd
    struct cmd *c = new_execmd(args);

    while (tokens[*pos]) {
        if (strcmp(tokens[*pos], "<") == 0) {
            (*pos)++;
            char *file = tokens[(*pos)++];
            c = new_redircmd(c, file, O_RDONLY, 0); // stdin <- file
        } else if (strcmp(tokens[*pos], ">") == 0) {
            (*pos)++;
            char *file = tokens[(*pos)++];
            c = new_redircmd(c, file, O_WRONLY | O_CREAT | O_TRUNC, 1); // stdout -> file   
        } else if (strcmp(tokens[*pos], "&") == 0) {
            (*pos)++;
            c = new_backgroundcmd(c);
        } else {
            break;
        }
    }

    return c;
}

struct cmd *parse_pipe(char *tokens[], int *pos) {
    // parse left of "|"
    struct cmd *c = parse_exec(tokens, pos);
    while (tokens[*pos] && strcmp(tokens[*pos], "|") == 0) {
        (*pos)++;  // skip "|"
        // parse right of "|"
        struct cmd *right = parse_exec(tokens, pos);
        c = new_pipecmd(c, right);
    }
    return c;
}

struct cmd *parse_line(char *tokens[], int *pos) {
    // parse left of ";"
    struct cmd *c = parse_pipe(tokens, pos);
    while (tokens[*pos] && strcmp(tokens[*pos], ";") == 0) {
        (*pos)++;
        // parse right of ";"
        struct cmd *right = parse_pipe(tokens, pos);
        c = new_listcmd(c, right);
    }
    return c;
}

// Function to parse the command string into a command structure
struct cmd *parse_cmd(const char *buffer) {
    char *buf = strdup(buffer);
    char *tokens[MAX_TOKENS];

    int ntokens = tokenize(buf, tokens);

    int pos = 0;
    struct cmd *c = parse_line(tokens, &pos);
    
    free(buf);
    return c;
}



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