/**
 * ls √
 * pwd
 * cd
 * mkdir
 * touch √
 * cat √
 * ps
 * clear
 * echo √
 * tab 键补齐
*/

#include "stdlib.h"

int argc = 0;
char argv[100][100];

static const char* builtinCommands[] = {
    "pwd", "cd", "mkdir", "ps", "clear", "ls", "touch", "echo", "cat", "whoami"
};
static const char* mkdirOptions[] = { "-p" };
static const char* lsOptions[] = { "-a" };

static void print_prompt();
static Boolean is_command_prefix(const char* command, const char* prefix);
static void complete_input(char* line, int* length, int capacity);
static void read_command_line(char* line, int capacity);

static void parse_command(const char* line) {
    argc = 0;
    memset(argv, 0, sizeof(argv));

    int argLength = 0;
    for (int i = 0; line[i] != '\0' && argc < 100; i++) {
        if (line[i] == ' ' || line[i] == '\t') {
            if (argLength > 0) {
                argc++;
                argLength = 0;
            }
        } else if (argLength < 99) {
            argv[argc][argLength++] = line[i];
        }
    }

    if (argLength > 0 && argc < 100) {
        argc++;
    }
}

static void print_prompt() {
    char* username = "JackHu";
    char* hostname = "JackHu-PC";
    printf("%s@%s:%s$ ", username, hostname, PWD());
}

static Boolean is_command_prefix(const char* command, const char* prefix) {
    for (int i = 0; prefix[i] != '\0'; i++) {
        if (command[i] != prefix[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

static int token_start(const char* line, int length) {
    int start = length;
    while (start > 0 && line[start - 1] != ' ' && line[start - 1] != '\t') {
        start--;
    }
    return start;
}

static int word_index(const char* line, int tokenStart) {
    int words = 0;
    Boolean inWord = FALSE;
    for (int i = 0; i < tokenStart; i++) {
        if (line[i] == ' ' || line[i] == '\t') {
            inWord = FALSE;
        } else if (!inWord) {
            words++;
            inWord = TRUE;
        }
    }
    return words;
}

static void first_word(const char* line, char* command, int capacity) {
    int i = 0;
    int k = 0;
    while (line[i] == ' ' || line[i] == '\t') {
        i++;
    }
    while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t' && k < capacity - 1) {
        command[k++] = line[i++];
    }
    command[k] = '\0';
}

static void append_text(char* line, int* length, int capacity, const char* text) {
    while (*text != '\0' && *length < capacity - 1) {
        line[(*length)++] = *text++;
    }
    line[*length] = '\0';
}

static void print_completion_list(const char* line, const char* candidates[], int candidateCount,
                                  const char* prefix) {
    printf("\n");
    for (int i = 0; i < candidateCount; i++) {
        if (is_command_prefix(candidates[i], prefix)) {
            printf("%s  ", candidates[i]);
        }
    }
    printf("\n");
    print_prompt();
    printf("%s", line);
}

static void complete_from_candidates(char* line, int* length, int capacity, int start,
                                     const char* candidates[], int candidateCount,
                                     Boolean appendSpace) {
    const char* prefix = line + start;
    const char* match = NULL;
    int matches = 0;
    int commonLength = 0;

    for (int i = 0; i < candidateCount; i++) {
        if (is_command_prefix(candidates[i], prefix)) {
            if (match == NULL) {
                match = candidates[i];
                while (match[commonLength] != '\0') {
                    commonLength++;
                }
            } else {
                int j = 0;
                while (j < commonLength && match[j] == candidates[i][j]) {
                    j++;
                }
                commonLength = j;
            }
            matches++;
        }
    }

    if (matches == 0) {
        return;
    }

    int prefixLength = *length - start;
    if (commonLength > prefixLength) {
        char suffix[128];
        int suffixLength = commonLength - prefixLength;
        for (int i = 0; i < suffixLength; i++) {
            suffix[i] = match[prefixLength + i];
        }
        suffix[suffixLength] = '\0';
        printf("%s", suffix);
        append_text(line, length, capacity, suffix);
    }

    if (matches == 1 && appendSpace && *length < capacity - 1) {
        line[(*length)++] = ' ';
        line[*length] = '\0';
        printf(" ");
    } else if (matches > 1) {
        print_completion_list(line, candidates, candidateCount, line + start);
    }
}

static int split_path_completions(char* raw, const char* candidates[], int capacity) {
    int count = 0;
    char* current = raw;
    for (int i = 0; raw[i] != '\0' && count < capacity; i++) {
        if (raw[i] == '\n') {
            raw[i] = '\0';
            candidates[count++] = current;
            current = raw + i + 1;
        }
    }
    return count;
}

static void complete_path(char* line, int* length, int capacity, int start) {
    char raw[1024];
    const char* candidates[32];
    Size matchCount = RequestPathCompletions(line + start, raw, sizeof(raw));
    int candidateCount = split_path_completions(raw, candidates,
                                                sizeof(candidates) / sizeof(candidates[0]));
    if (matchCount == 0 || candidateCount == 0) {
        return;
    }

    complete_from_candidates(line, length, capacity, start, candidates, candidateCount, FALSE);
}

static void complete_input(char* line, int* length, int capacity) {
    int start = token_start(line, *length);
    int index = word_index(line, start);

    if (index == 0) {
        complete_from_candidates(line, length, capacity, start, builtinCommands,
                                 sizeof(builtinCommands) / sizeof(builtinCommands[0]), TRUE);
        return;
    }

    char command[32];
    first_word(line, command, sizeof(command));
    if (index == 1 && line[start] == '-' && strcmp(command, "mkdir") == 0) {
        complete_from_candidates(line, length, capacity, start, mkdirOptions,
                                 sizeof(mkdirOptions) / sizeof(mkdirOptions[0]), TRUE);
        return;
    }
    if (index == 1 && line[start] == '-' && strcmp(command, "ls") == 0) {
        complete_from_candidates(line, length, capacity, start, lsOptions,
                                 sizeof(lsOptions) / sizeof(lsOptions[0]), TRUE);
        return;
    }

    complete_path(line, length, capacity, start);
}

static void read_command_line(char* line, int capacity) {
    int length = 0;
    line[0] = '\0';

    while (TRUE) {
        char ch = GetChar();
        if (ch == '\n') {
            return;
        }

        if (ch == '\b' || ch == 0x7F) {
            if (length > 0) {
                line[--length] = '\0';
            }
            continue;
        }

        if (ch == '\t') {
            complete_input(line, &length, capacity);
            continue;
        }

        if (ch >= ' ' && length < capacity - 1) {
            line[length++] = ch;
            line[length] = '\0';
        }
    }
}

int is_empty(char* name) {
    for (int i = 0; name[i]; i++) {
        if (name[i] != ' ' && name[i] != '\t' && name[i] != '\n') {
            return 0;
        }
    }

    return 1;
}

void shell() {
    while (1) {
        print_prompt();

        char line[1024];
        read_command_line(line, sizeof(line));

        parse_command(line);

        if (argc == 0) {
            continue;
        }
        
        char* cmd = argv[0]; // 命令
        if (strcmp(cmd, "pwd") == 0) {
            if (argc == 1) {
                printf("%s\n", PWD());
            } else {
                printf("pwd: too many arguments\n");
            }
        } else if (strcmp(cmd, "cd") == 0) {
            if (argc <= 2) {
                ChangeDir(argc == 2 ? argv[1] : "");
            } else {
                printf("usage: cd [path]\n");
            }
        } else if (strcmp(cmd, "mkdir") == 0) {
            if (argc == 2) {
                MakeDir(argv[1], "");
            } else if (argc == 3 && strcmp(argv[1], "-p") == 0) {
                MakeDir(argv[2], "-p");
            } else {
                printf("usage: mkdir [-p] path\n");
            }
        } else if (strcmp(cmd, "ps") == 0) {
            if (argc == 1) {
                ShowProcesses();
            } else {
                printf("ps: too many arguments\n");
            }
        } else if (strcmp(cmd, "clear") == 0) {
            if (argc == 1) {
                ClearScreen();
            } else {
                printf("clear: too many arguments\n");
            }
        } else if (strcmp(cmd, "ls") == 0) {
            /**
             * ls、ls path、ls -a path
            */
            if (argc == 1) {
                ListDir(".", "");
            }else if (argc == 2) {
                ListDir(argv[1], "");
            }else if (argc == 3) {
                if (strcmp(argv[1], "-a") == 0) {
                    ListDir(argv[2], argv[1]);
                }else {
                    printf("[ls] unknown option: %s\n", argv[1]);
                }
            }
        } else if (strcmp(cmd, "touch") == 0) {
            /**
             * touch file，在当前目录下创建一个文件
            */
            if (argc == 2) {
                if (is_empty(argv[1])) {
                    printf("unsupported file: empty filename!\n");
                    continue;
                }
                TouchFile(argv[1]);
            } else {
                printf("unsupport used of touch!\n");
            }
        } else if (strcmp(cmd, "echo") == 0) {
            /**
             * echo text > path，只支持写入当前目录下的文件
            */
            if (argc == 4) {
                if (strcmp(argv[2], ">") == 0) {
                    char* text = argv[1];
                    char* name = argv[3];
                    WriteToFile(text, name);
                } else {
                    printf("unsupport used of echo!\n");
                }
            }else {
                printf("unsupport used of echo!\n");
            }
        } else if (strcmp(cmd, "cat") == 0) {
            /**
             * cat filename
            */
           if (argc == 2) {
                BrowserFile(argv[1]);
                printf("\n");
           } else {
                printf("unsupport used of cat!\n");
           }
        } else if (strcmp(cmd, "whoami") == 0) {
            printf("root\n");
        } else {
            printf("Unknown command: %s\n", cmd);
        }
    }
}
