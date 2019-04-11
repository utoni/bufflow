#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CRYPT_PROLOGUE \
    { \
        asm volatile goto("jmp %l0          \n" \
            : : : : cr_prologue); \
        asm volatile( \
            ".byte 0xDE, 0xC0, 0xDE, 0xC0;  \n" \
        ); \
    } \
    cr_prologue: {
#define CRYPT_EPILOGUE \
    } { \
        asm volatile goto("jmp %l0          \n" \
            : : : : cr_epilogue); \
        asm volatile( \
            ".byte 0xFE, 0xCA, 0xFE, 0xCA;  \n" \
        ); \
    } \
    cr_epilogue: \
    asm volatile("nop;  \n");

typedef enum crypt_return {
    CRET_ERROR, CRET_PROLOGUE, CRET_EPILOGUE
} crypt_return;


static int crypted_fn(int arg0, char *arg1, void *arg2)
{
    CRYPT_PROLOGUE
    printf("I'm decrypted ..\n");
    for (int i = 0; i < 32; ++i)
        printf("%d ", i);
    puts("");
    CRYPT_EPILOGUE

    return 0x66;
}

static void crypted_fn2(void)
{
    CRYPT_PROLOGUE
    printf("Another decrypted fn..\n");
    CRYPT_EPILOGUE
}

static crypt_return crypt_func(void *fn_start)
{
    enum crypt_return cret = CRET_ERROR;
    uint8_t *fnbuf = (uint8_t *) fn_start;
    const uint32_t prologue_marker = 0xC0DEC0DE;
    const uint32_t epilogue_marker = 0xCAFECAFE;

    printf("Fn: %p\n", fnbuf);
    for (int i = 0; i < 0x100; ++i) {
        if (cret == CRET_ERROR &&
            *(uint32_t *) &fnbuf[i] == prologue_marker)
        {
            printf("Prologue Marker: %p\n", &fnbuf[i]);
            cret = CRET_PROLOGUE;
        } else
        if (cret == CRET_PROLOGUE &&
            *(uint32_t *) &fnbuf[i] == epilogue_marker)
        {
            printf("Epilogue Marker: %p\n", &fnbuf[i]);
            cret = CRET_EPILOGUE;
            break;
        }
    }

    return cret;
}

static void printHexBuf(uint8_t *buf, size_t siz, size_t chars_per_line)
{
    for (int i = 0; i < siz; ++i) {
        printf("%02X ", buf[i]);
        if ((i+1) % chars_per_line == 0)
            printf("\n");
    }
    printf("\n");
}

int main(void)
{
    printf("crypted_fn:\n");
    printHexBuf((uint8_t *)crypted_fn, 32, 16);
    printf("crypted_fn2:\n");
    printHexBuf((uint8_t *)crypted_fn2, 32, 16);

    printf("crypt_func:\n");
    crypt_func((void *)crypted_fn);
    crypt_func((void *)crypted_fn2);

    printf("\noutput:\n");
    printf("crypted_fn: 0x%X\n", crypted_fn(0, NULL, NULL));
    crypted_fn2();
}
