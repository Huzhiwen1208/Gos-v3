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

        // 读取字符，解析命令和参数
        memset(argv, 0, sizeof(argv));
        argc = 0;
        char ch;
        char temp[1024];
        memset(temp, 0, 1024);
        int k = 0;
        do {
            ch = GetChar();
            if (ch == ' ') {
                strcpy(argv[argc++], temp, strlen(temp));
                memset(temp, 0, 1024);
                k = 0;
            }else {
                temp[k++] = ch;
            }
        } while (ch != '\n');
        temp[k-1] = '\0';
        strcpy(argv[argc++], temp, strlen(temp));
        
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
        } else {
            printf("Unknown command: %s\n", cmd);
        }
    }
}
