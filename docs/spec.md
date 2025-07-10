# flowsh Specification

## 1. Project Overview
A proejct to build a minimal UNIX-style shell in C with:
- Read-eval-print loop
- Built-in commands with: `cd`, `exit`, `pwd`, `echo`
- External command execution through `fork()` and `execvp()`
- I/O redirection: `<`, `>`, `>>`
- Pipelines: `|`
- ASCII visualizer that snapshots shell's file descriptor table and shows how `fd` works

## 2. Core Features

1. **Shell**
- Prompt, parsing input into commands
- Execution of built-ins and external programs

2. **CMDs**
- `cd [dir]`
- `exit [n]`
- `pwd`
- `echo [args]`

3. **Redirections**
- `< filename`  
- `> filename` (truncate)  
- `>> filename` (append) 

4. **Pipelines**
- Basic pipeline: `cmd1 | cmd2`
- Chained pipelines: `cmd1 | cmd2 | ...`

5. **Visualizer**
- Snapshot `fd` table before/after redirection operations
- ASCII output diagram

## 3. Command Grammer
```peg
# Input line: one or more pipelines seperated by semicolons
Line       <- Pipeline ( ";" Pipeline )*

# Pipeline: one ore more commands seperated by pipes
Pipeline   <- Command ( "|" Command )*

# Command: one or more words, with zero or more redirections
SimpleCmd  <- Word+ Redirect*

# Redirection operators
Redirect   <- "<"  Word    # stdin from file
            / ">>" Word    # stdout append to file
            / ">"  Word    # stdout truncate to file

# A word: any run of characters except whitespace or shell metachars
Word       <- [^ \t\r\n|&;<>]+

```


