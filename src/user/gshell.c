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
 * whoami
 * ipaddr
*/

#include "stdlib.h"

int argc = 0;
char argv[100][100];

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
        char* username = "JackHu";
        char* hostname = "JackHu-PC";
        char* path = PWD();

        printf("%s@%s:%s$ ", username, hostname, path);

        char line[1024];
        Read(line, sizeof(line) - 1);
        parse_command(line);

        if (argc == 0) {
            continue;
        }
        
        char* cmd = argv[0]; // 命令
        if (strcmp(cmd, "ls") == 0) {
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
