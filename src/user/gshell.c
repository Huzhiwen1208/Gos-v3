#include "stdlib.h"
#include "../kernel/common/file_op.h"

#define SHELL_LINE_SIZE 512
#define SHELL_MAX_ARGS 16

/* Keep command storage outside the one-page user stack. */
static char commandLine[SHELL_LINE_SIZE];
static char* arguments[SHELL_MAX_ARGS];

static void usage(char* command, char* syntax) {
    printf("%s: usage: %s\n", command, syntax);
}

/* Split in place, accepting repeated whitespace and quoted arguments. */
static int parseArguments(char* line) {
    int count = 0;
    char* input = line;
    char* output = line;
    while (*input) {
        while (*input == ' ' || *input == '\t') input++;
        if (!*input) break;
        if (count == SHELL_MAX_ARGS) {
            printf("shell: too many arguments (maximum %d)\n", SHELL_MAX_ARGS);
            return -1;
        }
        arguments[count++] = output;
        char quote = 0;
        while (*input) {
            char ch = *input++;
            if (quote) {
                if (ch == quote) quote = 0;
                else *output++ = ch;
            } else if (ch == '\'' || ch == '"') {
                quote = ch;
            } else if (ch == ' ' || ch == '\t') {
                break;
            } else {
                *output++ = ch;
            }
        }
        if (quote) {
            printf("shell: unterminated quote\n");
            return -1;
        }
        *output++ = '\0';
    }
    return count;
}

/* Validate all operands/options before the first filesystem mutation. */
Boolean ShellParseFileOperation(int argc, char** argv, u32* flags,
                                char** paths, int* pathCount) {
    if (argc < 1 || argc > SHELL_MAX_ARGS || !argv || !flags || !paths || !pathCount)
        return FALSE;
    char* command = argv[0];
    Boolean remove = strcmp(command, "rm") == 0;
    Boolean copy = strcmp(command, "cp") == 0;
    Boolean move = strcmp(command, "mv") == 0;
    if (!remove && !copy && !move) return FALSE;
    *flags = 0;
    *pathCount = 0;
    Boolean options = TRUE;
    for (int i = 1; i < argc; i++) {
        char* argument = argv[i];
        if (options && strcmp(argument, "--") == 0) {
            options = FALSE;
            continue;
        }
        if (options && argument[0] == '-' && argument[1]) {
            for (int j = 1; argument[j]; j++) {
                char option = argument[j];
                if ((remove || copy) && (option == 'r' || option == 'R'))
                    *flags |= FILE_RECURSIVE;
                else if (option == 'f')
                    *flags = (*flags | FILE_FORCE) & ~(FILE_INTERACTIVE | FILE_NO_CLOBBER);
                else if (option == 'i')
                    *flags = (*flags | FILE_INTERACTIVE) & ~(FILE_FORCE | FILE_NO_CLOBBER);
                else if (!remove && option == 'n')
                    *flags = (*flags | FILE_NO_CLOBBER) & ~(FILE_FORCE | FILE_INTERACTIVE);
                else if (option == 'v')
                    *flags |= FILE_VERBOSE;
                else if (remove && option == 'd')
                    *flags |= FILE_DIRECTORY;
                else if (!remove && option == 'T')
                    *flags |= FILE_NO_TARGET_DIR;
                else {
                    printf("%s: invalid option: -%c\n", command, option);
                    return FALSE;
                }
            }
        } else {
            if (!argument[0]) {
                printf("%s: empty path is not allowed\n", command);
                return FALSE;
            }
            paths[(*pathCount)++] = argument;
        }
    }
    if (remove) {
        if (!*pathCount && !(*flags & FILE_FORCE)) {
            usage(command, "rm [-rRfidv] [--] path ...");
            return FALSE;
        }
    } else {
        if (*pathCount < 2) {
            usage(command, copy ? "cp [-rRfinvT] [--] source ... target" : "mv [-finvT] [--] source ... target");
            return FALSE;
        }
        if (*pathCount > 2) {
            if (*flags & FILE_NO_TARGET_DIR) {
                printf("%s: -T cannot be used with multiple sources\n", command);
                return FALSE;
            }
            *flags |= FILE_TARGET_DIRECTORY;
        }
    }
    return TRUE;
}

static void executeFileOperation(int argc) {
    char* paths[SHELL_MAX_ARGS];
    int count;
    u32 flags;
    if (!ShellParseFileOperation(argc, arguments, &flags, paths, &count)) return;
    Boolean remove = strcmp(arguments[0], "rm") == 0;
    Boolean copy = strcmp(arguments[0], "cp") == 0;
    int sources = remove ? count : count - 1;
    for (int i = 0; i < sources; i++) {
        if (remove) RemoveFilePath(paths[i], flags);
        else if (copy) CopyFilePath(paths[i], paths[count - 1], flags);
        else MoveFilePath(paths[i], paths[count - 1], flags);
    }
}

void ShellExecuteLine(char* line) {
    int argc = parseArguments(line);
    if (argc <= 0) return;
    char* cmd = arguments[0];

    if (strcmp(cmd, "rm") == 0 || strcmp(cmd, "cp") == 0 || strcmp(cmd, "mv") == 0) {
        executeFileOperation(argc);
    } else if (strcmp(cmd, "pwd") == 0) {
        if (argc != 1) usage(cmd, "pwd");
        else printf("%s\n", PWD());
    } else if (strcmp(cmd, "mkdir") == 0) {
        char* option = "";
        int first = 1;
        if (argc > 1 && strcmp(arguments[1], "-p") == 0) {
            option = "-p";
            first++;
        }
        if (first == argc) {
            usage(cmd, "mkdir [-p] path ...");
        } else {
            for (int i = first; i < argc; i++) {
                if (arguments[i][0] == '-') {
                    printf("mkdir: invalid option: %s\n", arguments[i]);
                    break;
                }
                MakeDir(arguments[i], option);
            }
        }
    } else if (strcmp(cmd, "cd") == 0) {
        if (argc > 2) usage(cmd, "cd [path]");
        else ChangeDir(argc == 1 ? "~" : arguments[1]);
    } else if (strcmp(cmd, "clear") == 0) {
        if (argc != 1) usage(cmd, "clear");
        else ClearScreen();
    } else if (strcmp(cmd, "ls") == 0) {
        char* option = "";
        char* path = ".";
        int next = 1;
        if (argc > 1 && strcmp(arguments[1], "-a") == 0) {
            option = "-a";
            next++;
        }
        if (next < argc && arguments[next][0] == '-') {
            printf("ls: invalid option: %s\n", arguments[next]);
        } else if (argc > next + 1) {
            usage(cmd, "ls [-a] [path]");
        } else {
            if (next < argc) path = arguments[next];
            ListDir(path, option);
        }
    } else if (strcmp(cmd, "touch") == 0) {
        if (argc != 2 || arguments[1][0] == '\0') usage(cmd, "touch filename");
        else TouchFile(arguments[1]);
    } else if (strcmp(cmd, "echo") == 0) {
        if (argc == 4 && strcmp(arguments[2], ">") == 0) {
            WriteToFile(arguments[1], arguments[3]);
        } else {
            usage(cmd, "echo text > filename");
        }
    } else if (strcmp(cmd, "cat") == 0) {
        if (argc != 2) usage(cmd, "cat path");
        else {
            BrowserFile(arguments[1]);
            printf("\n");
        }
    } else if (strcmp(cmd, "whoami") == 0) {
        printf("root\n");
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

void shell() {
    while (1) {
        printf("JackHu@JackHu-PC:%s$ ", PWD());
        int length = 0;
        int excess = 0;
        for (;;) {
            char ch = GetChar();
            if (ch == '\n' || ch == '\r') {
                printf("\n");
                break;
            }
            if (ch == '\b') {
                if (excess > 0) excess--;
                else if (length > 0) {
                    length--;
                    printf("\b");
                }
            } else if ((ch >= ' ' && ch <= '~') || ch == '\t') {
                if (length < SHELL_LINE_SIZE - 1 && excess == 0) {
                    commandLine[length++] = ch;
                    /* Tab is a visible delimiter; one backspace removes it. */
                    printf("%c", ch == '\t' ? ' ' : ch);
                } else {
                    excess++;
                }
            }
        }
        commandLine[length] = '\0';
        if (excess) {
            printf("shell: input too long (maximum %d characters)\n", SHELL_LINE_SIZE - 1);
            continue;
        }
        ShellExecuteLine(commandLine);
    }
}
