#include <stdio.h>

static char *gets(char *string, size_t size)
{
    char *out = fgets(string, size, stdin);
    if (out)
        string[strcspn(string, "\n")] = '\0';
    return out;
}

void interactive_mode()
{
    for (;;) {
        int err = -1;
        *command = '\0';
        *prog = '\0';
        *arg = '\0';

        printf(BOLD GRN "%s@%s" RESET ":", username, machname);
        if (initialized) {
            printf(BOLD BLU);
            pwd(&files);
            printf(RESET);
            printf("$ ");
        } else {
            printf("> ");
        }

        fflush(stdout);

        while (!gets(command, sizeof(command)));

        sscanf(command, "%s %s", prog, arg);
        if (!strcmp(prog, "mkfs")) {
            mkfs(&files);
            initialized = true;
        } else if (!strcmp(prog, "quit")) {
            exit(EXIT_SUCCESS);
        } else if (!*prog) {
        } else{
            printf("%s: command not found\n", prog);
        }

        if (!err)
            puts("Error: command returned 0");

    }
}

int main(int argc, char **argv)
{

}
